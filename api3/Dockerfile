FROM golang:1.23-alpine

WORKDIR /app

COPY main.go .

RUN go build -o api3 main.go

EXPOSE 8003

CMD ["./api3"]
