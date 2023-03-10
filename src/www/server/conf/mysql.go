package conf
type DbConfig struct {
	DriverName string
	Dsn string
	ShowSql bool
	ShowExecTime bool
	MaxIdle int
	MaxOpen int
}

var Db = map[string]DbConfig{
	"db1": {
		DriverName: "mysql",
		Dsn:        "root:T-Flash-Dev@tcp(tflash.pwnsky.com:10086)/tflash?charset=utf8mb4&parseTime=true&loc=Local",
		ShowSql:    true,
		ShowExecTime:    false,
		MaxIdle:    10,
		MaxOpen:    200,
	},
}
