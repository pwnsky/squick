<template>
  <a-card :bordered="false">
    <a-steps class="steps" :current="current">
      <a-step :title="'生成序列号'" />
    </a-steps>
    <div class="content">
      <step1 v-if="current === 0" @nextStep="nextStep"></step1>
    </div>
  </a-card>
</template>

<script>
import Step1 from './Step1'

export default {
  name: 'View',
  components: {Step1},
  data () {
    return {
      current: 0
    }
  },
  computed: {
    desc() {
      return this.$t('pageDesc')
    }
  },
  methods: {
    nextStep () {
      if (this.current < 2) {
        //this.current += 1
      }
    },
    prevStep () {
      if (this.current > 0) {
        this.current -= 1
      }
    },
    finish () {
      this.current = 0
    }
  }
}
</script>

<style lang="less" scoped>
  .steps{
    max-width: 950px;
    margin: 16px auto;
  }
</style>
