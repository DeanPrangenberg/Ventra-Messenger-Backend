package logs

import "log"

var Debug = true

func DebugLog(format string, v ...interface{}) {
	if Debug {
		log.Printf("[DEBUG] "+format, v...)
	}
}
