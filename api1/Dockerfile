FROM golang:1.23-alpine

WORKDIR /app

COPY main.go .

RUN go build -o api1 main.go

EXPOSE 8001

CMD ["./api1"]
