FROM python:3

COPY requirements.txt requirements.txt

RUN pip install -r requirements.txt 

COPY waf /waf

COPY conf/start.sh /start.sh

RUN chmod +x /start.sh /waf

COPY src /app

WORKDIR /app

CMD [ "/start.sh" ]
