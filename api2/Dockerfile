FROM golang:1.23-alpine

WORKDIR /app

COPY main.go .

RUN go build -o api2 main.go

EXPOSE 8002

CMD ["./api2"]
