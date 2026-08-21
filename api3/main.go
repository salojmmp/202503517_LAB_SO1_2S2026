package main
import (
	"encoding/json"
	"fmt"
	"io"
	"log"
	"net/http"
	"time"
)
const PORT = "8003"
const CARNET = "202503517"
const VM = "VM2"
const SHARED_IP = "192.168.122.80"

func healthHandler(w http.ResponseWriter, r *http.Request) {
	response := map[string]interface{}{
		"status":    "UP",
		"message":   "API3 is Ready",
		"timestamp": time.Now().Format(time.RFC3339),
		"VM":        VM,
		"carnet":    CARNET,
	}
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(response)
}

func callAPI(w http.ResponseWriter, r *http.Request, targetAPI string, targetIP string, targetPort string) {
	url := fmt.Sprintf("http://%s:%s/health", targetIP, targetPort)
	resp, err := http.Get(url)
	if err != nil {
		response := map[string]interface{}{
			"apiname":    targetAPI,
			"message":    fmt.Sprintf("ERROR: The %s located on the VM is not working", targetAPI),
			"connection": false,
			"carnet":     CARNET,
		}
		w.Header().Set("Content-Type", "application/json")
		json.NewEncoder(w).Encode(response)
		return
	}
	defer resp.Body.Close()
	body, _ := io.ReadAll(resp.Body)
	var healthResp map[string]interface{}
	json.Unmarshal(body, &healthResp)
	if healthResp["status"] == "UP" {
		response := map[string]interface{}{
			"apiname":    targetAPI,
			"message":    fmt.Sprintf("The %s located on the %s is working", targetAPI, healthResp["VM"]),
			"connection": true,
			"carnet":     CARNET,
		}
		w.Header().Set("Content-Type", "application/json")
		json.NewEncoder(w).Encode(response)
	}
}

func main() {
	http.HandleFunc("/health", healthHandler)
	http.HandleFunc("/api3/"+CARNET+"/call-api1", func(w http.ResponseWriter, r *http.Request) {
		callAPI(w, r, "API1", SHARED_IP, "8001")
	})
	http.HandleFunc("/api3/"+CARNET+"/call-api2", func(w http.ResponseWriter, r *http.Request) {
		callAPI(w, r, "API2", SHARED_IP, "8002")
	})
	log.Fatal(http.ListenAndServe(":"+PORT, nil))
}
