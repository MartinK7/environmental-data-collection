FROM python:3.11-slim-bullseye

USER root

RUN groupadd -g 1000 edcflask && useradd -u 1000 -g edcflask -s /bin/bash -m edcflask

USER edcflask

RUN pip install flask plotly

USER root

WORKDIR /app

COPY app/ .

RUN chown -R edcflask:edcflask /app

USER edcflask

CMD ./edc_flask_server.sh

