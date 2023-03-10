package lang

import (
	"github.com/pwnsky/t-flash-server/conf"
)

var langMap = map[string]map[string]string{
"cn": cn,
"en": en,
}

func Get(value string) string{
	langKey := ""
	if conf.Cfg.Language=="" {
		langKey = "cn"
	}else{
		langKey = conf.Cfg.Language
	}
	if msg,ok :=langMap[langKey][value];ok {
		return msg
	}
	return value
}