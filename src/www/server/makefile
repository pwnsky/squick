TARGET := t-flash-server
#CGO_ENABLED=0 go build -a -ldflags '-extldflags "-static"' .

$(TARGET): main.go
	go build
	#CGO_ENABLED=0 GOOS=linux go build
	#CGO_ENABLED=0 GOOS=linux go build -a -ldflags '-extldflags "-static"'
run:
	sudo ./$(TARGET) http :80
clean:
	$(RM) $(TARGET)

static : main.go
	CGO_ENABLED=0 GOOS=linux go build -a -ldflags '-extldflags "-static"' -buildvcs=false
