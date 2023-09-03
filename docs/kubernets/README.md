# K8s



## 环境搭建

本次k8s集群采用kind来进行搭建

### Linux

设置 go 代理

```
echo "export GO111MODULE=on" >> ~/.profile
echo "export GOPROXY=https://goproxy.cn" >> ~/.profile
source ~/.profile
```

设置path

```
echo "export PATH=$PATH:~/go/bin" >> ~/.profile
source ~/.profile
```

下载kind

```
go install sigs.k8s.io/kind@v0.20.0 && kind create cluster
```



### Windows

设置go代理

```
 SETX GO111MODULE on    
 go env -w GOPROXY=https://goproxy.cn,direct
 SETX GOPROXY https://goproxy.cn,direct
```

下载kind

```
go install sigs.k8s.io/kind@v0.20.0 && kind create cluster
```



### 