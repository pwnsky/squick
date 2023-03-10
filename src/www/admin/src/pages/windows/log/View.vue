<template>
  <div>
    <a-card :bordered="false">
      <div style="display: flex; flex-wrap: wrap">
          <head-info title="客户端日志" :bordered="true"/>
      </div>
    </a-card>
    <a-card
      style="margin-top: 24px"
      :bordered="false"
      title="最近100个日志"
    >
      <div slot="extra">
        <a-input-search style="margin-left: 16px; width: 272px;" />
      </div>

      <a-list size="large" :pagination="{showSizeChanger: true, showQuickJumper: true, pageSize: number_of_itmes_per_page, total: item_array.length}">
        <a-list-item :key="item" v-for="item in item_array">
          <a-list-item-meta
            :description="'设备uuid: ' + item.uuid"
            
          >
            <a slot="title" >{{ item.message }}</a>
          </a-list-item-meta>
         
          <div class="list-content">
            <div class="list-content-item">
              <span>访问ip</span>
              <p>{{ item.ip }}</p>
            </div>
            <div class="list-content-item">
              <span>创建时间</span>
              <p>{{ item.time }}</p>
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
import {handleGetClientLog} from "@/services/windows/log";
export default {
  name: 'ShowLog',
  components: {HeadInfo},
  data () {
    return {
      number_of_itmes_per_page: 10,
      number_of_itmes:0,
      item_array: {},
      form: this.$form.createForm(this),
    }
  },
  beforeMount() { // 初始化函数
    //this.number_of_itmes_per_page = 1;
    this.item_array = [{serial_code:'1143434, ', created_time : "1", expired_time : "2"}]
    var data = {begin:100, end: -1, status : -1}
    handleGetClientLog(data).then(res => {
            const body = res.data
            if (body.code === 200) {
              //this.$message.success('生成成功', 3)
              this.item_array = body.data;
              this.number_of_itmes = body.data.length
              for(var i = 0; i < body.data.length; i++){
                body.data[i].time = new Date(body.data[i].time).Format("yyyy-MM-dd hh:mm:ss");
              }
              this.number_of_itmes_per_page =  body.data.length;
            } else if (body.code === 400) {
              
              //this.$message.error('生成失败: ' + updateRes.msg, 3)
            }
          })
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
