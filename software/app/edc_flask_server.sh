while true; do
    # Start the Python app in the foreground
    python3 /app/edc_flask_server.py | tee -a log.log

    # If the app exited with an error, wait 10 seconds before retrying
    if [ $? -ne 0 ]; then
        echo "App exited with error, restarting in 10 seconds..."
        sleep 10
    else
        # App exited normally
        echo "App exited normally, restarting in 10 seconds..."
        sleep 10
    fi
done

