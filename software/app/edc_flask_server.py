# Module Imports
import datetime
import gzip
import json
import math
import sqlite3
import sys
import threading
import struct
import os
import time
import requests

from flask import Flask, render_template, request, make_response, send_file, request, Response

# Open configuration file
with open('config.json') as f:
	server_configuration = json.load(f)

database_lock = threading.Lock()
cameraimage_lock = threading.Lock()
cameraimage = None


####################
# System		   #
####################

def handle_exception(exc_type, exc_value, exc_traceback):
	print(f"Unhandled exception: {exc_type.__name__}: {exc_value}")
	print("Restarting application...")
	sys.exit(1)


####################
# SQL-Lite section #
####################

def save_value(cursor, time_sec, table_name, value):
	query = f"INSERT INTO {table_name} (time_sec, value) VALUES (?, ?)"
	values = (time_sec, value)
	try:
		# Insert value into table
		cursor.execute(query, values)
	except sqlite3.Error as e:
		# Print error
		print(f"Error inserting value to table!\n{e}")
		print(f"Creating table {table_name}")
		# Check if table exists, create if not
		cursor.execute(f"CREATE TABLE {table_name} (id INTEGER PRIMARY KEY, time_sec TIMESTAMP, value FLOAT)")
		# Insert value into table (again)
		cursor.execute(query, values)


def database_connect():
	try:
		conn = sqlite3.connect(server_configuration['sqlite']['database'])
		cursor = conn.cursor()
		return conn, cursor
	except sqlite3.Error as e:
		print(f"Error connecting to SQLite server: {e}")
		sys.exit(1)


def database_close(conn, cursor):
	cursor.close()
	conn.close()


def sql_time_from_unix_timestamp(unix_timestamp):
	timestamp = datetime.datetime.utcfromtimestamp(int(unix_timestamp)).strftime("%Y-%m-%d %H:%M:%S")
	return timestamp


def commit_data(unix_timestamp, measurement):
	with database_lock:
		# Connect to SQLite server
		conn, cursor = database_connect()

		sqltimestamp = sql_time_from_unix_timestamp(unix_timestamp)

		for m in measurement:
			save_value(cursor, sqltimestamp, m[0], m[1])

		# Commit the changes to the database
		conn.commit()

		# Close the cursor and connection
		database_close(conn, cursor)

		# Upload data to wunderground
		try:
			url = 'https://weatherstation.wunderground.com/weatherstation/updateweatherstation.php'
			#url = 'http://127.0.0.1:5000'
			station_id = server_configuration['wunderground']['station_id']
			if not station_id:
				return
			key_password = server_configuration['wunderground']['key_password']
			if not key_password:
				return
			bindings = server_configuration['wunderground']['bindings']
			wunderground_data = ''
			for m in measurement:
				for binding in bindings:
					source = binding['source']
					if not source:
						continue
					if m[0] == source:
						destination = binding['destination']
						if not destination:
							break
						formula = binding['formula']
						if formula:
							value = eval(formula.replace('{}', str(m[1])))
						else:
							value = str(m[1])
						if len(wunderground_data) > 0:
							wunderground_data = wunderground_data + '&'
						wunderground_data = wunderground_data + m[0] + '=' + str(value)
						break
			endpoint_url = f"{url}?ID={station_id}&PASSWORD={key_password}&{wunderground_data}&action=updateraw&dateutc={sqltimestamp}"
			print(f"Wunderground - URL: '{endpoint_url}'")
			response = requests.get(endpoint_url)
			if response.status_code == 200:
				print("Wunderground - Data uploaded successfully.")
			else:
				print("Wunderground - Error uploading data.")
		except:
				print("Wunderground - Exception!")



##################
# EDC parser     #
##################

# Returns True if the input string s contains only alphabetic characters and no whitespace, False otherwise.
def is_alphabetic_no_space(s):
	for char in s:
		if ord('a') <= ord(char) <= ord('z'):
			continue
		if ord('A') <= ord(char) <= ord('Z'):
			continue
		if ord('0') <= ord(char) <= ord('9'):
			continue
		if ord(char) == ord('_'):
			continue
		return False
	return True


def is_number_or_float(s):
	try:
		float(s)
		return True
	except:
		return False


def parse_edcascii_string(s):
	try:
		# Check if the message starts with "EDCASCII"
		if not s[:9].decode().startswith("EDCASCII:"):
			return False
		# Remove the "EDCASCII:" prefix
		s = s[9:].decode()
		# Tokenize
		tokens = s.split(",")
		measurement = []
		for token in tokens:
			name, value = token.split("=")
			if is_alphabetic_no_space(name) and is_number_or_float(value):
				allowed_tables = server_configuration['sqlite']['allowed_tables']
				for allowed_table in allowed_tables:
					if allowed_table['table_name'] == name:
						measurement.append([name, float(value)])
						break
		# Do not return empty measurement
		if not measurement:
			return False
		return measurement
	except:
		print("This is not valied EDCASCII")
		return False


def process_edcbinary(unix_timestamp, s):
	try:
		# Check if the message starts with "EDCBINARY:"
		if not s[:10].decode().startswith("EDCBINARY:"):
			return False

		# Unpack the binary data
		edcbinary, type_, size, checksum = struct.unpack('<10sHII', s[:20])
		pixels = s[20:]

		# Convert edcbinary to a string
		edcbinary = edcbinary.decode('ascii')

		# Check the values
		if edcbinary != 'EDCBINARY:':
			print('Error: Invalid edcbinary')
			return False
		elif type_ != 1:
			print('Error: Invalid type')
			return False
		elif size != len(s):
			print(f"Error: Invalid size {size} != {len(s)}")
			return False
		else:
			# Calculate the checksum
			message_without_checksum = s[:16] + b'\x00\x00\x00\x00' + pixels
			calculated_checksum = sum(message_without_checksum) & 0xffffffff
			if checksum != calculated_checksum:
				print('Error: Invalid checksum')
				return False
		print('JPEG DETECTED!')

		# create directory if it doesn't exist
		directory = 'camera'
		if not os.path.exists(directory):
			os.makedirs(directory)

		# write byte array to file
		with open(os.path.join(directory, f"{unix_timestamp}.jpg"), 'wb') as f:
			f.write(pixels)

		with cameraimage_lock:
			global cameraimage
			cameraimage = pixels
		return True
	except:
		print("This is not valied EDCBINARY")
		return False


def print_measurement(measurement):
	# Determine the maximum width of each column
	col_widths = [max(len(str(row[i])) for row in measurement) for i in range(len(measurement[0]))]

	# Print the top border
	print("+" + "+".join("-" * (w + 2) for w in col_widths) + "+")

	# Print the measurement contents
	for row in measurement:
		print("| " + " | ".join(str(row[i]).ljust(col_widths[i]) for i in range(len(row))) + " |")

		# Print a separator between rows
		print("+" + "+".join("-" * (w + 2) for w in col_widths) + "+")


##################
# Flask website  #
##################

app = Flask(__name__, template_folder='templates')


def compress_response(request, data):
	if 'gzip' not in request.headers.get('Accept-Encoding', ''):
		return data
	compressed_data = gzip.compress(data.encode('utf-8'))  # compress the data
	response = make_response(compressed_data)
	response.headers['Content-Encoding'] = 'gzip'
	response.headers['Content-Length'] = len(compressed_data)
	return response


@app.route('/upload', methods=['POST'])
def app_page_upload():
	unix_timestamp = int(time.time())
	print(str(f"Received header: '{request.headers}'")[:128])
	print(str(f"Received body: '{request.data}'")[:128])
	if request.headers.get('X-Message-Age'):
		unix_timestamp = unix_timestamp - int(request.headers.get('X-Message-Age'))
	if process_edcbinary(unix_timestamp, request.data):
		return 'EDCASCII:response=okay'
	# Parse EDC TCP format
	measurement = parse_edcascii_string(request.data)
	# If the format is ok print measurement
	if measurement:
		print_measurement(measurement)
		commit_data(unix_timestamp, measurement)
		return 'EDCASCII:response=okay'
	return 'EDCASCII:response=error'


@app.route('/tables')
def app_page_tables():
	with database_lock:
		# Connect to SQLite server
		conn, cursor = database_connect()

		# get a list of all the tables in the database
		cursor.execute("SELECT name FROM sqlite_master WHERE type='table'")
		tables = cursor.fetchall()

		tables_json = []

		allowed_tables = server_configuration['sqlite']['allowed_tables']

		# create a graph for each table in the database
		for allowed_table in allowed_tables:
			for table in tables:
				if allowed_table['table_name'] == table[0]:
					tables_json.append(allowed_table)
					break

		tables_json = json.dumps(tables_json)

		# Close the cursor and connection
		database_close(conn, cursor)

		return compress_response(request, tables_json)


@app.route('/cameraimage.jpg')
def app_page_cameraimage():
	with cameraimage_lock:
		if not cameraimage:
			return None

		# Check if client supports gzip encoding
		accept_encoding = request.headers.get('Accept-Encoding', '')

		if 'gzip' in accept_encoding:
			# Compress the response with gzip
			response = Response(gzip.compress(cameraimage), mimetype='image/jpeg')
			response.headers.set('Content-Encoding', 'gzip')
		else:
			# Send the response without gzip compression
			response = Response(cameraimage, mimetype='image/jpeg')
	return response


@app.route('/data')
def app_page_data():
	data_json = "{}"

	table_name = request.args.get('table_name')
	start_time = request.args.get('start_time')
	end_time = request.args.get('end_time')

	if not table_name:
		return compress_response(request, data_json)
	if not start_time:
		return compress_response(request, data_json)
	if not end_time:
		return compress_response(request, data_json)

	allow = False
	allowed_tables = server_configuration['sqlite']['allowed_tables']
	for allowed_table in allowed_tables:
		if allowed_table['table_name'] == table_name:
			allow = True
			break
	if not allow:
		return compress_response(request, data_json)

	start_time = sql_time_from_unix_timestamp(start_time)
	end_time = sql_time_from_unix_timestamp(end_time)

	with database_lock:
		# Connect to SQLite server
		conn, cursor = database_connect()

		try:
			# Count how many there are values
			cursor.execute(
				"SELECT COUNT(*) FROM '%s' WHERE time_sec BETWEEN '%s' AND '%s'" % (table_name, start_time, end_time))
			count = cursor.fetchall()[0][0]

			# calculate decimation
			query_limit = server_configuration['sqlite']['query_limit']
			decimation_factor = float(count) / float(query_limit)
			decimation_factor = int(math.ceil(decimation_factor))
			if decimation_factor <= 0:
				decimation_factor = 1

			# get decimated data
			cursor.execute(
				"SELECT time_sec, value FROM '%s' WHERE time_sec BETWEEN '%s' AND '%s' AND id %% '%d' = 0 ORDER BY time_sec ASC" % (
					table_name, start_time, end_time, decimation_factor))
			data = cursor.fetchall()

			data_json = json.dumps(data)
		except sqlite3.Error as e:
			print(f"Error server: {e}")
			database_close(conn, cursor)
			return compress_response(request, data_json)

		# Close the cursor and connection
		database_close(conn, cursor)

		return compress_response(request, data_json)


@app.route('/')
def app_page_index():
	with open('templates/plotly-1.58.5.min.js', 'r') as f:
		plotly_script = f.read()
		# render the template with the figure
		return compress_response(request, render_template('index.html', plotly=plotly_script))


########
# Main #
########

if __name__ == '__main__':
	# System
	sys.excepthook = handle_exception

	# Start the Flask app in debug mode in the main thread
	app.run(host=server_configuration['flask']['host'], port=server_configuration['flask']['port'])
