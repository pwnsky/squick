<template>
  <div>
    <a-form  :form="form"  style="max-width: 500px; margin: 40px auto 0;">
      <a-form-item
        :label="'序列号类型'"
        :labelCol="{span: 7}"
        :wrapperCol="{span: 17}"
      >
        <a-input-group :compact="true" style="display: inline-block; vertical-align: middle">
          <a-select defaultValue="0" style="width: 100px">
            <a-select-option value="0">{{$t('标准')}}</a-select-option>
            <a-select-option value="1">{{$t('测试')}}</a-select-option>
          </a-select>
          
        </a-input-group>

      </a-form-item>
      <a-form-item
        :label="'有效天数'"
        :labelCol="{span: 7}"
        :wrapperCol="{span: 17}"
      >
        <a-input
          value="180"
          placeholder="有效天数： 1 ~ 360 天"
          v-decorator="['days', {rules: [{ required: true, message: '有效天数', whitespace: true}]}]"
         />
      </a-form-item>
      <a-form-item
        :label="'增加数量'"
        :labelCol="{span: 7}"
        :wrapperCol="{span: 17}"
      >
        <a-input prefix=""
        value="5"
        placeholder="增加数量： 0 ~ 100 个"
        v-decorator="['amount', {rules: [{ required: true, message: '数量', whitespace: true}]}]"
         />
      </a-form-item>
      <a-form-item :wrapperCol="{span: 17, offset: 7}">
        <a-button type="primary" @click="serial_code_gen">{{$t('生成')}}</a-button>
      </a-form-item>
    </a-form>
  </div>
</template>

<script>
import {handleGenerateSerialCode} from "@/services/windows/serial_code";
export default {
  name: 'Step1',
  data () {
    return {
      form: this.$form.createForm(this)
    }
  },
  created: function () {
    //this.server_name = "localhost"
    // 获取信息
    this.form.setFieldsValue({
        amount: '5',
        days: '180',
      })
  },
  methods: {
    
    serial_code_gen() {
        const data = {
            amount : Number(this.form.getFieldValue('amount')),
            days : Number(this.form.getFieldValue('days')),
            type : Number('0')
          }

          if(isNaN(data.amount) || isNaN(data.days) || data.amount <= 0 || data.amount > 100 || data.days <= 0 || data.days > 365) {
            alert("填写参数错误")
            return
          }

          handleGenerateSerialCode(data).then(res => {
            //
            const body = res.data
            if (body.code === 200) {
              alert('生成成功')
              //this.$message.success('生成成功', 3)
            } else if (body.code === 400) {
              alert('生成失败')
              //this.$message.error('生成失败: ' + updateRes.msg, 3)
            }
          })
    }
  }
}
</script>

<style scoped>

</style>
