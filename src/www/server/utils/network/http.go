package network

import (
	"net/http"
	"io/ioutil"
	"strings"
)

func HttpPost(url string, contentType string, body []byte) (string, error) {
    res, err := http.Post(url, contentType, strings.NewReader(string(body)))
    if err != nil {
        return "", err
    }
    defer res.Body.Close()
    content, err := ioutil.ReadAll(res.Body)
    if err != nil {
        return "", err
    }
    return string(content), nil
}

func HttpGet(url string) (string, error) {
    response, err := http.Get(url)
	if err != nil {
		return "", err
	}
	defer response.Body.Close()
	body, err2 := ioutil.ReadAll(response.Body)
	if err2 != nil {
		return "", err
	}
	return string(body), err

}