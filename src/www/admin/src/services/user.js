//import {SIGN_IN, SIGN_UP, CAPTCHA_INIT, CAPTCHA_IMAGE, ROUTES, CHECK_AUTH} from '@/services/api'
import {request, METHOD, removeAuthorization} from '@/utils/request'
import * as api from '@/services/api'
// import { CAPTCHA_INIT } from './api'

/**
 * 登录处理
 * @param name 账户名
 * @param password 账户密码
 * @param capcha 验证码
 * @returns {Promise<AxiosResponse<T>>}
 */
export async function handleSignin(account, password, captcha_code, captcha_id) {
  return request(api.SIGN_IN, METHOD.POST, {
    account: account,
    password: password,
    captcha_code: captcha_code,
    captcha_id: captcha_id,
  })
}

export async function handleSignup(account, password, auth_code, captcha_code, captcha_id) {
  return request(api.SIGN_UP, METHOD.POST, {
    account: account,
    password: password,
    auth_code:auth_code,
    captcha_code: captcha_code,
    captcha_id:captcha_id,
  })
}


// 获取验证码信息
export async function getCapatchaID() {
  return request(api.CAPTCHA_INIT, METHOD.GET)
}

// 返回验证码图片URL
export function getCapatchaImageURL(captcha_id) {
  return api.CAPTCHA_IMAGE + '?id=' + captcha_id
}

// 检查自己是否登录
export async function checkAuth() {
  return request(api.CHECK_AUTH, METHOD.GET)
}

/**
 * 退出登录
 */
export function logout() {
  localStorage.removeItem(process.env.VUE_APP_ROUTES_KEY)
  localStorage.removeItem(process.env.VUE_APP_PERMISSIONS_KEY)
  localStorage.removeItem(process.env.VUE_APP_ROLES_KEY)
  removeAuthorization()
}
export default {
  handleSignin,
  logout,
  //getRoutesConfig
}

export async function getRoutesConfig() {
  return request(api.ROUTES, METHOD.GET)
}
