<template>
  <common-layout>
    <div class="top">
      <div class="header">
        <img alt="logo" class="logo" src="@/assets/img/logo.png" />
        <span class="title">{{systemName}}</span>
      </div>
      <div class="desc">T-Flash</div>
    </div>
    <div class="login">
      <a-form @submit="onSubmit" :form="form">
        <a-tabs size="large" :tabBarStyle="{textAlign: 'center'}" style="padding: 0 2px;">
          <a-tab-pane tab="登录" key="1">
            <a-alert type="error" :closable="true" v-show="error" :message="error" showIcon style="margin-bottom: 24px;" />
            <a-form-item>
              <a-input
                autocomplete="autocomplete"
                size="large"
                placeholder="账号"
                v-decorator="['account', {rules: [{ required: true, message: '请输入账户名', whitespace: true}]}]"
              >
                <a-icon slot="prefix" type="user" />
              </a-input>
            </a-form-item>
            <a-form-item>
              <a-input
                size="large"
                placeholder="密码"
                autocomplete="autocomplete"
                type="password"
                v-decorator="['password', {rules: [{ required: true, message: '请输入密码', whitespace: true}]}]"
              >
                <a-icon slot="prefix" type="lock" />
              </a-input>
            </a-form-item>

            <a-form-item>
              <a-row :gutter="8" style="margin: 0 -4px">
                <a-col :span="16">
                  <a-input
                size="large"
                placeholder="验证码"
                autocomplete="autocomplete"
                type="captcha"
                v-decorator="['captcha', {rules: [{ required: true, message: '请输入验证码', whitespace: true}]}]"
              >
                    <a-icon slot="prefix" type="mail" />
                  </a-input>
                </a-col>
                <a-col :span="8" style="padding-left: 4px;height:30px">
                  <img style="width:100%;height:100%" alt="captcha_image" class="captcha_image" :src="captcha_image_url" @click="getCapatchaImage"/>
                </a-col>
              </a-row>
            </a-form-item>

            <div>
              <a-checkbox :checked="true" >自动登录</a-checkbox>
              <a style="float: right">忘记密码</a>
            </div>

            <a-form-item>
              <a-button  @click="signin" style="width: 100%;margin-top: 24px" size="large" htmlType="submit" type="primary">登录</a-button>
            </a-form-item>

          </a-tab-pane>

          <!-- </a-form>
          <a-form> -->
          <a-tab-pane tab="注册" key="2">
            <a-alert type="error" :closable="true" v-show="error" :message="error" showIcon style="margin-bottom: 24px;" />
            <a-form-item>
              <a-input
                autocomplete="autocomplete"
                size="large"
                placeholder="授权码"
                v-decorator="['auth_code', {rules: [{ required: true, message: '授权码', whitespace: true}]}]"
              >
                <a-icon slot="prefix" type="user" />
              </a-input>
            </a-form-item>
            <a-form-item>
              <a-input
                autocomplete="autocomplete"
                size="large"
                placeholder="账号"
                v-decorator="['account', {rules: [{ required: true, message: '账号', whitespace: true}]}]"
              >
                <a-icon slot="prefix" type="user" />
              </a-input>
            </a-form-item>
            <a-form-item>
              <a-input
                size="large"
                placeholder="密码"
                autocomplete="autocomplete"
                type="password"
                v-decorator="['password', {rules: [{ required: true, message: '请输入密码', whitespace: true}]}]"
              >
                <a-icon slot="prefix" type="lock" />
              </a-input>
            </a-form-item>

            <a-form-item>
              <a-row :gutter="8" style="margin: 0 -4px">
                <a-col :span="16">
                  <a-input
                size="large"
                placeholder="验证码"
                autocomplete="autocomplete"
                type="captcha"
                v-decorator="['captcha', {rules: [{ required: true, message: '请输入验证码', whitespace: true}]}]"
                >
                  <a-icon slot="prefix" type="mail" />
                  </a-input>
                </a-col>
                <a-col :span="8" style="padding-left: 4px;height:30px">
                  <img style="width:100%;height:100%" alt="captcha_image" class="captcha_image" :src="captcha_image_url" @click="getCapatchaImage" />
                </a-col>
              </a-row>
            </a-form-item>

            <a-form-item>
              <a-button @click="signup" style="width: 100%;margin-top: 24px" size="large" htmlType="submit" type="primary"  >注册</a-button>
            </a-form-item>
          </a-tab-pane>

        </a-tabs>
        </a-form>


        <!-- <div>
          其他登录方式
          <a-icon class="icon" type="alipay-circle" />
          <a-icon class="icon" type="taobao-circle" />
          <a-icon class="icon" type="weibo-circle" />
          <router-link style="float: right" to="/dashboard/workplace" >注册账户</router-link>
        </div> -->

    </div>
  </common-layout>
</template>

<script>
import CommonLayout from '@/layouts/CommonLayout'// getRoutesConfig
import {handleSignin, handleSignup, getCapatchaID, getCapatchaImageURL, checkAuth} from '@/services/user'
import {setAuthorization} from '@/utils/request'
//import {loadRoutes} from '@/utils/routerUtil'
import {mapMutations} from 'vuex'

export default {
  beforeMount() { // 初始化函数
    // 先检查一下自己是否已经登录过。
    checkAuth().then(res => {
            // 注册响应
            const signInRes = res.data
            if(signInRes.code == 200) { // 已经登录过，直接跳转
              this.$message.success('你已经登录过', 3)
              this.$router.push('/dashboard/workplace')
            }else {
              //没有登录过
              this.getCapatchaImage() // 获取验证码图片
            }
          })
    // mounted
    

  },
  name: 'Login',
  components: {CommonLayout},
  data () {
    return {
      is_signin: false,
      is_sginup: false,
      error: '',
      captcha_image_url: '',
      captcha_id: '',
      form: this.$form.createForm(this)
    }
  },
  computed: {
    systemName () {
      return this.$store.state.setting.systemName
    }
  },
  methods: {
    ...mapMutations('account', ['setUser', 'setPermissions', 'setRoles']),
    onSubmit (e) { // 点击登录事件
      //alert(e.target.tagName)
      e.preventDefault()

    },
    signin(e) {
      e.preventDefault()
      this.form.validateFields((err) => {
        if (!err) {
          this.is_signin = true
          const account = this.form.getFieldValue('account')
          const password = this.form.getFieldValue('password')
          const captcha = this.form.getFieldValue('captcha')
          handleSignin(account, password, captcha, this.captcha_id).then(res => {
            // 登录响应
            console.log(res)
            const signInRes = res.data
            if(signInRes.code == 200) {
              //alert('登录成功')
              //const {user, permissions, roles} = signInRes.data
              //this.setUser(user)
              //this.setPermissions(permissions)
              //this.setRoles(roles)
              var date = new Date(new Date().getTime() + 24 * 60 * 60 * 1000)
              setAuthorization( //设置验证的Cookie
              {
                access_token: signInRes.access_token,
                refrash_token: signInRes.refrash_token,
                user_id: signInRes.user_id,
                expires: date
              })

              //alert('yeah: ')
              //loadRoutes(routesConfig)
              this.$router.push('/dashboard/workplace')
              this.$message.success(signInRes.msg, 3)
            }else if(signInRes.code == 400){
              this.$message.success('登录失败：' + signInRes.msg, 5)
              this.getCapatchaImage()
            }
          })

          //alert('点击登录' + account + password + captcha)
        }
      })

    },
    //点击注册按钮事件
    signup(e){
      e.preventDefault()
      this.form.validateFields((err) => {
        if (!err) {
          this.is_signin = true
          const account = this.form.getFieldValue('account')
          const password = this.form.getFieldValue('password')
          const captcha = this.form.getFieldValue('captcha')
          const auth_code = this.form.getFieldValue('auth_code')
          if(account.length < 6 || password.length < 6) {
              this.$message.success('账号或密码长度必须要大于等于6', 3)
              return
          }
          handleSignup(account, password, auth_code, captcha, this.captcha_id).then(res => {
            // 注册响应
            const signInRes = res.data
            if(signInRes.code == 200) {
              this.$message.success('注册成功', 3)
            }else if(signInRes.code == 400){
              this.$message.success('注册失败：' + signInRes.msg, 3)
              this.getCapatchaImage()
            }
          })

        }
      })
    },
    //
    getCapatchaImage() {
        //e.preventDefault()
        getCapatchaID().then(res => {
        // 获取到验证码id
        console.log(res)
        this.captcha_id = res.data.data
        this.captcha_image_url = getCapatchaImageURL(this.captcha_id)
        //console.log(getCapatchaImageURL(captcha_id))
      })
    },

  }
}
</script>

<style lang="less" scoped>
  .common-layout{
    .top {
      text-align: center;
      .header {
        height: 44px;
        line-height: 44px;
        a {
          text-decoration: none;
        }
        .logo {
          height: 44px;
          vertical-align: top;
          margin-right: 16px;
        }
        .title {
          font-size: 33px;
          color: @title-color;
          font-family: 'Myriad Pro', 'Helvetica Neue', Arial, Helvetica, sans-serif;
          font-weight: 600;
          position: relative;
          top: 2px;
        }
      }
      .desc {
        font-size: 14px;
        color: @text-color-second;
        margin-top: 12px;
        margin-bottom: 40px;
      }
    }
    .login{
      width: 368px;
      margin: 0 auto;
      @media screen and (max-width: 576px) {
        width: 95%;
      }
      @media screen and (max-width: 320px) {
        .captcha-button{
          font-size: 14px;
        }
      }
      .icon {
        font-size: 24px;
        color: @text-color-second;
        margin-left: 16px;
        vertical-align: middle;
        cursor: pointer;
        transition: color 0.3s;

        &:hover {
          color: @primary-color;
        }
      }
    }
  }
</style>
