package main

import (
	"bytes"
	"io"
	"log"
	"mime"
	"net/http"
	"regexp"
	"strings"
)

const backendURL = "http://127.0.0.1:8000"
const backendHost = "127.0.0.1:8000"

var blockedIPs = map[string]bool{
	"1.1.1.1": true,
}

var sqlInjectionPattern = regexp.MustCompile(`(?i)(union.*select|select.*from|insert.*into|update.*set|delete.*from|drop\s+table|--|#|\*\/|\/\*)`)

var rcePattern = regexp.MustCompile(`(?i)(\b(?:os|exec|system|eval|passthru|shell_exec|phpinfo|popen|proc_open|pcntl_exec|assert)\s*\(.+\))`)

var hotfixPattern = regexp.MustCompile(`(?i)(select)`)

var blockedUserAgents = []string{
	"sqlmap",
	"nmap",
	"curl",
}

func isBlockedIP(ip string) bool {
	return blockedIPs[ip]
}

func isMaliciousRequest(r *http.Request) bool {
	for key, values := range r.URL.Query() {
		for _, value := range values {
			if sqlInjectionPattern.MatchString(value) {
				log.Printf("阻止 SQL 注入: 参数 %s=%s", key, value)
				return true
			}
			if rcePattern.MatchString(value) {
				log.Printf("阻止 RCE 攻击: 参数 %s=%s", key, value)
				return true
			}
			if hotfixPattern.MatchString(value) {
				log.Printf("参数 %s=%s", key, value)
				return true
			}
		}
	}

	if r.Method == http.MethodPost {
		ct := r.Header.Get("Content-Type")
		mediaType, _, err := mime.ParseMediaType(ct)
		if err != nil {
			log.Printf("解析 Content-Type 失败: %v", err)
			return true
		}
		if mediaType == "multipart/form-data" {
			if err := r.ParseMultipartForm(65535); err != nil {
				log.Printf("解析 POST 参数失败: %v", err)
				return true
			}
		} else {
			if err := r.ParseForm(); err != nil {
				log.Printf("解析 POST 参数失败: %v", err)
				return true
			}
		}

		for key, values := range r.PostForm {
			log.Printf("POST 参数 %s=%v", key, values)
			for _, value := range values {
				if sqlInjectionPattern.MatchString(value) {
					log.Printf("阻止 SQL 注入: POST 参数 %s=%s", key, value)
					return true
				}
				if rcePattern.MatchString(value) {
					log.Printf("阻止 RCE 攻击: POST 参数 %s=%s", key, value)
					return true
				}
				if hotfixPattern.MatchString(value) {
					log.Printf("POST 参数 %s=%s", key, value)
					return true
				}

			}
		}
	}
	return false
}

func isBlockedUserAgent(userAgent string) bool {
	for _, blocked := range blockedUserAgents {
		if strings.Contains(strings.ToLower(userAgent), blocked) {
			return true
		}
	}
	return false
}

func reverseProxyHandler(w http.ResponseWriter, r *http.Request) {
	clientIP := r.RemoteAddr
	if isBlockedIP(clientIP) {
		http.Error(w, "Forbidden", http.StatusForbidden)
		log.Printf("阻止的 IP: %s", clientIP)
		return
	}

	bodyBytes, err := io.ReadAll(r.Body)

	if err != nil {
		http.Error(w, "Bad Request", http.StatusBadRequest)
		return
	}

	r.Body = io.NopCloser(bytes.NewBuffer(bodyBytes))

	if isMaliciousRequest(r) {
		http.Error(w, "Malicious request detected", http.StatusForbidden)
		return
	}

	if isBlockedUserAgent(r.UserAgent()) {
		http.Error(w, "Forbidden User-Agent", http.StatusForbidden)
		log.Printf("阻止的 User-Agent: %s", r.UserAgent())
		return
	}

	proxyReq, err := http.NewRequest(r.Method, backendURL+r.RequestURI, bytes.NewBuffer(bodyBytes))
	if err != nil {
		http.Error(w, "Bad Gateway", http.StatusBadGateway)
		return
	}
	proxyReq.Header = r.Header

	client := &http.Client{
		CheckRedirect: func(req *http.Request, via []*http.Request) error {
			return http.ErrUseLastResponse
		},
	}

	resp, err := client.Do(proxyReq)
	if err != nil {
		http.Error(w, "Bad Gateway", http.StatusBadGateway)
		return
	}
	defer resp.Body.Close()

	for key, values := range resp.Header {
		for _, value := range values {
			if key == "Location" {
				value = strings.Replace(value, backendHost, r.Host, -1)
			}
			w.Header().Add(key, value)
		}
	}
	w.WriteHeader(resp.StatusCode)
	io.Copy(w, resp.Body)
}

func main() {
	http.HandleFunc("/", reverseProxyHandler)
	log.Println("Listen on 0.0.0.0:8080")
	log.Fatal(http.ListenAndServe(":8080", nil))
}
