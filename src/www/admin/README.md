# T-Flash后台前端代码



https://github.com/iczer/vue-antd-admin

https://iczer.gitee.io/vue-antd-admin-docs

## 使用

### clone
```bash
$ git clone https://github.com/iczer/vue-antd-admin.git
```
### yarn
```bash
$ yarn install
$ yarn serve
```
### or npm
```
$ npm install
$ npm run serve
```









```
npm ERR! code ERESOLVE
npm ERR! ERESOLVE unable to resolve dependency tree
npm ERR!
npm ERR! While resolving: vue-antd-admin@0.7.4
npm ERR! Found: vue@2.6.14
npm ERR! node_modules/vue
npm ERR!   vue@"^2.6.11" from the root project
npm ERR!
npm ERR! Could not resolve dependency:
npm ERR! peer vue@">=3.0.3" from @ant-design/icons-vue@6.1.0
npm ERR! node_modules/@ant-design/icons-vue
npm ERR!   @ant-design/icons-vue@"^6.1.0" from the root project
npm ERR!
npm ERR! Fix the upstream dependency conflict, or retry
npm ERR! this command with --force, or --legacy-peer-deps
npm ERR! to accept an incorrect (and potentially broken) dependency resolution.
npm ERR!
npm ERR! See C:\Users\i0gan\AppData\Local\npm-cache\eresolve-report.txt for a full report.

```

出现以上错误时，运行：

```
npm install --force
```

