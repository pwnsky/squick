<template>
  <a-card :body-style="{padding: '24px 32px'}" :bordered="false">
    <a-form :form="form" @submit="onSubmit">
      <a-form-item
          :label="$t('服务器名称')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="服务器名称"
            v-decorator="['server_name', {rules: [{ required: true, message: '服务器名称', whitespace: true}]}]"
        />
      </a-form-item>

      <a-form-item
          :label="$t('主后台服务器URL')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="主后台服务器URL"
            v-decorator="['main_server_url', {rules: [{ required: true, message: '主后台服务器URL', whitespace: true}]}]"
        />
      </a-form-item>

      <a-form-item
          :label="$t('舌诊体制识别系统服务器URL')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="舌诊体制识别系统服务器URL"
            v-decorator="['client_sztzsb_url', {rules: [{ required: true, message: '舌诊体制识别系统服务器URL', whitespace: true}]}]"
        />
      </a-form-item>

      <a-form-item
          :label="$t('中医资讯服务器URL')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="中医资讯服务器URL"
            v-decorator="['client_zyzx_url', {rules: [{ required: true, message: '中医咨询服务器URL', whitespace: true}]}]"
        />
      </a-form-item>

      <a-form-item
          :label="$t('养生坊服务器URL')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="养生坊服务器URL"
            v-decorator="['client_ysf_url', {rules: [{ required: true, message: '养生坊服务器URL', whitespace: true}]}]"
        />
      </a-form-item>


       <a-form-item
          :label="$t('中医名家故事服务器URL')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="中医名家故事服务器URL"
            v-decorator="['client_zymjgs_url', {rules: [{ required: true, message: '中医名家故事服务器URL', whitespace: true}]}]"
        />
      </a-form-item>

       <a-form-item
          :label="$t('中医文化科普服务器URL')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="中医文化科普服务器URL"
            v-decorator="['client_zywhkp_url', {rules: [{ required: true, message: '中医文化科普服务器URL', whitespace: true}]}]"
        />
      </a-form-item>

      <a-form-item
          :label="$t('杏林轶事服务器URL')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="杏林轶事服务器URL"
            v-decorator="['client_xlys_url', {rules: [{ required: true, message: '杏林轶事服务器URL', whitespace: true}]}]"
        />
      </a-form-item>

      <a-form-item
          :label="$t('中医wiki服务器URL')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="中医wiki服务器URL"
            v-decorator="['client_zywiki_url', {rules: [{ required: true, message: '中医wiki服务器URL', whitespace: true}]}]"
        />
      </a-form-item>


      <a-form-item
          :label="$t('识药消消乐服务器URL')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="识药消消乐服务器URL"
            v-decorator="['client_syxxl_url', {rules: [{ required: true, message: '识药消消乐服务器URL', whitespace: true}]}]"
        />
      </a-form-item>

      <a-form-item
          :label="$t('健身气功服务器URL')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="健身气功服务器URL"
            v-decorator="['client_jsqg_url', {rules: [{ required: true, message: '健身气功服务器URL', whitespace: true}]}]"
        />
      </a-form-item>
      <a-form-item
          :label="$t('名家谈养生服务器URL')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="名家谈养生服务器URL"
            v-decorator="['client_mjtys_url', {rules: [{ required: true, message: '名家谈养生服务器URL', whitespace: true}]}]"
        />
      </a-form-item>
      
      <a-form-item
          :label="$t('客户端资源更新服务器')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="客户端资源更新服务器"
            v-decorator="['client_update_url', {rules: [{ required: true, message: '客户端资源更新服务器', whitespace: true}]}]"
        />
      </a-form-item>

      <a-form-item
          :label="$t('网站URL')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="网站URL"
            v-decorator="['client_www_url', {rules: [{ required: true, message: '网站URL', whitespace: true}]}]"
        />
      </a-form-item>

      <a-form-item
          :label="$t('后端接口授权秘钥')"
          :labelCol="{span: 7}"
          :wrapperCol="{span: 10}"
      >
        <a-input
            placeholder="后端接口授权秘钥"
            v-decorator="['auth_key', {rules: [{ required: true, message: '后端接口授权秘钥', whitespace: true}]}]"
        />
      </a-form-item>

      

      <a-form-item style="margin-top: 24px" :wrapperCol="{span: 10, offset: 7}">
        <a-button type="primary" @click="gatewayUpdate">{{$t('submit')}}</a-button>
        <a-button style="margin-left: 8px">{{$t('save')}}</a-button>
      </a-form-item>
    </a-form>
  </a-card>
</template>

<script>

import {handleGetGateway, handleUpdateGateway} from "@/services/common/gateway";

export default {
  name: 'Gateway',
  data () {
    return {
      err: '',
      is_submit: false,
      form: this.$form.createForm(this),
    }
  },
  created: function () {
    //this.server_name = "localhost"
    // 获取信息
    handleGetGateway().then(res => {
      const gateway = res.data.gateway
      this.form.setFieldsValue({
        auth_key: gateway.auth_key,
        client_www_url: gateway.client_www_url,
        client_zywiki_url: gateway.client_zywiki_url,
        server_name: gateway.server_name,
        main_server_url: gateway.client_main_url,
        client_update_url : gateway.client_update_url,
        client_sztzsb_url: gateway.client_sztzsb_url,
        client_syxxl_url: gateway.client_syxxl_url,
        client_jsqg_url: gateway.client_jsqg_url,
        client_zyzx_url: gateway.client_zyzx_url,
        client_mjtys_url: gateway.client_mjtys_url,
        client_ysf_url: gateway.client_ysf_url,
        client_xlys_url: gateway.client_xlys_url,
        client_zymjgs_url: gateway.client_zymjgs_url,
        client_zywhkp_url: gateway.client_zywhkp_url
      })
    })
  },
  methods:{
    onSubmit(e) {
      e.preventDefault()
    },
    gatewayUpdate(e) {
      e.preventDefault()
      this.form.validateFields((err) => {
        if (!err) {
          this.is_submit = true
          // const serverName = this.form.getFieldValue('')

          const gateway = {
            auth_key : this.form.getFieldValue('auth_key'),
            client_www_url : this.form.getFieldValue('client_www_url'),
            client_zywiki_url : this.form.getFieldValue('client_zywiki_url'),
            server_name : this.form.getFieldValue('server_name'),
            main_server_url : this.form.getFieldValue('main_server_url'),
            client_update_url : this.form.getFieldValue('client_update_url'),
            client_sztzsb_url : this.form.getFieldValue('client_sztzsb_url'),
            client_syxxl_url : this.form.getFieldValue('client_syxxl_url'),
            client_jsqg_url : this.form.getFieldValue('client_jsqg_url'),
            client_zyzx_url : this.form.getFieldValue('client_zyzx_url'),
            client_mjtys_url : this.form.getFieldValue('client_mjtys_url'),
            client_ysf_url : this.form.getFieldValue('client_ysf_url'),
            client_xlys_url : this.form.getFieldValue('client_xlys_url'),
            client_zymjgs_url : this.form.getFieldValue('client_zymjgs_url'),
            client_zywhkp_url: this.form.getFieldValue('client_zywhkp_url')
          }

          handleUpdateGateway(gateway).then(res => {
            const updateRes = res.data
            if (updateRes.code === 200) {
              this.$message.success('更新成功', 3)
            } else if (updateRes.code === 400) {
              this.$message.error('更新失败: ' + updateRes.msg, 3)
            }
          })

        }else{
          console.log(err)
        }
      })
    }
  },
  computed: {
    desc() {
      return this.$t('pageDesc')
    }
  }
}
</script>

<style scoped>

</style>
