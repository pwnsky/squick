<template>
  <a-card :bordered="false" class="search-form">
    <a-form :form="form">
     

      <form-row label="筛选">
        <a-row>
          <a-col :span="8">
            <a-form-item label="设备" :labelCol="{span: 6}" :wrapperCol="{span: 12}">
              <a-select placeholder="全部">
                <a-select-option value="1">全部</a-select-option>
                <a-select-option value="2">设备1</a-select-option>
                <a-select-option value="3">设备2</a-select-option>
              </a-select>
            </a-form-item>
          </a-col>
          <a-col :span="8">
            <a-form-item
              label="识别状态"
              :labelCol="{span: 6}"
              :wrapperCol="{span: 12}"
            >
              <a-select placeholder="全部">
                <a-select-option value="1">全部</a-select-option>
                <a-select-option value="2">已识别</a-select-option>
                <a-select-option value="3">未识别</a-select-option>
              </a-select>
            </a-form-item>
          </a-col>
        </a-row>
      </form-row>
    </a-form>
  </a-card>
</template>

<script>
import FormRow from '../../../../components/form/FormRow'


export default {
  name: 'SearchForm',
  components: {FormRow},
  data() {
    return {
      form: this.$form.createForm(this)
    }
  },
  methods: {
    lookMyself () {
      this.form.setFieldsValue({
        owner: '3'
      })
    }
  }
}
</script>

<style lang="less" scoped>
  .search-form{
    margin-bottom: 24px;
  }
</style>
