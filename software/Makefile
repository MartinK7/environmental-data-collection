all:
	docker stop edc_flask_server || true
	docker rm edc_flask_server || true
	docker build -t edc_flask_server_image .
	docker run -d --network="host" -t -i --name edc_flask_server edc_flask_server_image

