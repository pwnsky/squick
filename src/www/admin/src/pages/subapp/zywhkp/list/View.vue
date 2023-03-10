<template>
  <div>
    <a-card :bordered="false">
      <div style="display: flex; flex-wrap: wrap">
          <head-info title="视频列表" content="" :bordered="true"/>
      </div>
    </a-card>

    <a-card
      style="margin-top: 24px"
      :bordered="false"
      title="视频列表"
    >
      <div slot="extra">
        <a-input-search style="margin-left: 16px; width: 272px;" />
      </div>


      <a-list :grid='{ gutter: 24, xl: 4, lg: 3, md: 3, sm: 2, xs: 1 }'
      style="margin: 0 -8px">
        <a-list-item :key="item" v-for="item in item_array">

          <a-card>
          <img slot="cover" :src="item.image_url" width="384" height="216"/>
            <div >
              <span>标题</span>
              <p>{{ item.title }}</p>
            </div>

           <div >
              <span>描述</span>
              <p>{{ item.intro }}</p>
            </div>

            <div class="list-content-item">
              <span>创建时间</span>
              <p>{{ item.ctime }}</p>
            </div>

            <div class="list-content-item">
              <span>类别</span>
              <p>{{ typeIndeToString(item.type) }}</p>
            </div>

          <div slot="actions">
            <a @click="handleShow(item.id)">查看</a>
          </div>
          <div slot="actions">
            <a @click="handleEdit(item.id)">编辑</a>
          </div>
          <div slot="actions">
            <a @click="handleDelete(item.id)">删除</a>
          </div>

        </a-card>
            
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
import {handleVideoGetAll, handleVideoDelete } from "@/services/subapp/zywhkp";

export default {
  name: 'View',
  components: {HeadInfo},
  data () {
    return {
      number_of_itmes_per_page: 10,
      number_of_itmes:0,
      item_array: {},
      form: this.$form.createForm(this),
      title :"视频列表",
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
      if(status == null) {
        status = null;
      }

      //e.preventDefault()
      this.item_array = {}
      handleVideoGetAll().then(res => {
            const body = res.data
            if (body.code === 200) {
              console.log(body)
              this.$message.success('获取成功', 3)

              this.number_of_itmes = body.data.length
              
              for(var i = 0; i < body.data.length; i++){
                body.data[i].ctime  = new Date(body.data[i].ctime).Format("yyyy-MM-dd hh:mm:ss");
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
    },
    handleShow(id) {
      //e.preventDefault()
      var index = 0
      for(var i = 0; i < this.item_array.length; i++){
          if(this.item_array[i].id == id) {
              index = i
          }
      }
      this.item_array[index]
      
      
      window.open(this.item_array[index].video_url)
    },
    handleEdit(id) {
        this.$router.push('/sub_app/ysf/new?type=edit&id=' + id)
    },
    handleDelete(id) {

        handleVideoDelete(id).then(res => {
            const updateRes = res.data
            if (updateRes.code === 200) {
               this.$message.success('删除成功', 3)
               this.refrashView(0) 

             } else if (updateRes.code === 400) {
                            
                this.$message.error('删除失败: ' + updateRes.msg, 3)
               }
             })
    },
    typeIndeToString (type) {
    switch(type) {
      case 1: {
        return "默认"
      }
    }
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
