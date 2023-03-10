<template>
  <div>
    <a-card :bordered="false">
      <div style="display: flex; flex-wrap: wrap">
          <head-info :title="title" :content="number_of_itmes" :bordered="true"/>
      </div>
    </a-card>
    
    <a-card :bordered="false" class="search-form">
    <a-form :form="form">
      <form-row label="其他选项">
        <a-row>
          <a-col :span="8">
            <a-form-item label="查看类型" :labelCol="{span: 6}" :wrapperCol="{span: 12}">
              <a-select placeholder="已激活" 
              @change="refrashViewEvent($event,value)"
              >
                <a-select-option value="1" >已激活</a-select-option>
              </a-select>
            </a-form-item>
          </a-col>
        </a-row>
      </form-row>
    </a-form>
   </a-card>

    <a-card
      style="margin-top: 24px"
      :bordered="false"
      title="已激活设备信息"
    >
      <div slot="extra">
        <a-input-search style="margin-left: 16px; width: 272px;" />
      </div>

      <a-list size="large" :pagination="{showSizeChanger: true, showQuickJumper: true, pageSize: number_of_itmes_per_page, total: item_array.length}">
        <a-list-item :key="item" v-for="item in item_array">
          <a-list-item-meta
            description=""
          >
            <a slot="title" >设备</a>
          </a-list-item-meta>
          <div slot="actions">
            <a>删除</a>
          </div>
          <div class="list-content">
            <div class="list-content-item">
              <span>设备名称</span>
              <p>{{ item.name }}</p>
            </div>
            <div class="list-content-item">
              <span>激活时间</span>
              <p>{{ item.active_time }}</p>
            </div>
            <div class="list-content-item">
              <span>设备UUID</span>
              <p>{{ item.uuid }}</p>
            </div>
            <div class="list-content-item">
              <span>绑定序列号</span>
              <p>{{ item.serial_code }}</p>
            </div>
            <div class="list-content-item">
              <span>管理员</span>
              <p>{{ item.admin }}</p>
            </div>
            <div class="list-content-item">
              <span>管理员电话</span>
              <p>{{ item.phone }}</p>
            </div>
            <div class="list-content-item">
              <span>使用单位</span>
              <p>{{ item.company }}</p>
            </div>
            <div class="list-content-item">
              <span>地址</span>
              <p>{{ item.address }}</p>
            </div>
            <div class="list-content-item">
              <span>公网IP</span>
              <p>{{ item.ip }}</p>
            </div>
            <div class="list-content-item">
              <span>设备信息</span>
              <p>{{ item.system }}</p>
            </div>
            
            
          </div>
        </a-list-item>
      </a-list>


    </a-card>
  </div>
</template>

<script>

Date.prototype.Format = function(fmt)   
{ 
  var o = {   

    "M+" : this.getMonth()+1,                 //月份   
    "d+" : this.getDate(),                    //日
    "h+" : this.getHours(),                   //小时   
    "m+" : this.getMinutes(),                 //分   
    "s+" : this.getSeconds(),                 //秒   
    "q+" : Math.floor((this.getMonth()+3)/3), //季度   
    "S"  : this.getMilliseconds()             //毫秒   
  };   
  if(/(y+)/.test(fmt))   
    fmt=fmt.replace(RegExp.$1, (this.getFullYear()+"").substr(4 - RegExp.$1.length));   
  for(var k in o)   
    if(new RegExp("("+ k +")").test(fmt))   
  fmt = fmt.replace(RegExp.$1, (RegExp.$1.length==1) ? (o[k]) : (("00"+ o[k]).substr((""+ o[k]).length)));   
  return fmt;
};

import HeadInfo from '@/components/tool/HeadInfo'
import {handleGetClientDeviceInfo} from "@/services/windows/device";
export default {
  name: 'ActivatedDevices',
  components: {HeadInfo},
  data () {
    return {
      number_of_itmes_per_page: 10,
      number_of_itmes:0,
      item_array: {},
      form: this.$form.createForm(this),
      title :"已经激活设备",
    }
  },
  beforeMount() { // 初始化函数
    //this.number_of_itmes_per_page = 1;
    this.refrashView(0); // 默认是获取未注册的
  },
  methods: {
    onSubmit (e) { // 点击登录事件
      //alert(e.target.tagName)
      e.preventDefault()

    },
    refrashView(status) {
      //e.preventDefault()
      this.item_array = {}
      var data = {begin:1, amount: 50, status : Number(status)}
      handleGetClientDeviceInfo(data).then(res => {
            const body = res.data
            if (body.code === 200) {
              //this.$message.success('生成成功', 3)
              
              this.number_of_itmes = body.data.length
              for(var i = 0; i < body.data.length; i++){
                  body.data[i].active_time = new Date(body.data[i].active_time).Format("yyyy-MM-dd hh:mm:ss");
                  //body.data[i].expired_time = new Date(body.data[i].expired_time * 1000).Format("yyyy-MM-dd hh:mm:ss");
              }
              this.item_array = body.data;

              this.number_of_itmes_per_page =  body.data.length;
            } else if (body.code === 400) {
              
              //this.$message.error('生成失败: ' + updateRes.msg, 3)
            }
        })

    },
    refrashViewEvent(status) {
      //alert("ok" + status)
      this.refrashView(status)
    }
  },
}
</script>

<style lang="less" scoped>
  .list-content-item{
    color: @text-color-second;
    display: inline-block;
    vertical-align: middle;
    font-size: 14px;
    margin-left: 40px;
    span{
      line-height: 20px;
    }
    p{
      margin: 4px 0 0;
      line-height: 22px;
    }
  }
</style>
