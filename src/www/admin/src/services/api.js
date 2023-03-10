//跨域代理前缀
const API_PROXY_PREFIX='/api'
const BASE_URL = process.env.NODE_ENV === 'production' ? process.env.VUE_APP_API_BASE_URL : API_PROXY_PREFIX
//const BASE_URL = process.env.VUE_APP_API_BASE_URL
//const BASE_URL = "http://127.0.0.1:1024"   // 开发调试URL
//const BASE_URL = "https://tflash.pwnsky.com" // 发布URL
module.exports = {
  SIGN_IN: `${BASE_URL}/pub/admin/sign_in`,
  SIGN_UP: `${BASE_URL}/pub/admin/sign_up`,
  SIGN_OUT: `${BASE_URL}/admin/sign_out`,
  CHECK_AUTH: `${BASE_URL}/pub/admin/check_auth`,                          // 检查是否已经登录了

  ROUTES: `${BASE_URL}/api/pub/routes`,
  GOODS: `${BASE_URL}/api/auth/goods`,
  GOODS_COLUMNS: `${BASE_URL}/api/auth/columns`,

  CAPTCHA_INIT: `${BASE_URL}/pub/admin/captcha/init`,                  // 验证码初始化API
  CAPTCHA_IMAGE: `${BASE_URL}/pub/admin/captcha/image`,                // 验证码图像API

  GATEWAY_UPDATE: `${BASE_URL}/admin/windows/gateway/update`,      // 网关更新API
  GATEWAY_GET: `${BASE_URL}/admin/windows/gateway/get`,            // 网关信息获取

  CLIENT_XLYS_UPLOAD: `${BASE_URL}/admin/windows/xlys`,     // 杏林轶事文件上传API

  SERIAL_CODE_GENERATE: `${BASE_URL}/admin/windows/serial_code/generate`, // 序列号生成
  SERIAL_CODE_GET: `${BASE_URL}/admin/windows/serial_code/get`,           // 序列号获取
  CLIENT_LOG_GET: `${BASE_URL}/admin/windows/log/get`,             // 客户端日志获取
  CLIENT_DEVICE_GET: `${BASE_URL}/admin/windows/device/get`,       // 客户端设备信息获取



  SUBAPP_ZYZX_ARTICAL_ADD: `${BASE_URL}/admin/subapp/zyzx/artical/add`,  // 中医资讯添加文章
  SUBAPP_ZYZX_ARTICAL_GET_LIST: `${BASE_URL}/admin/subapp/zyzx/artical/getlist`,  // 中医资讯获取文章列表
  SUBAPP_ZYZX_ARTICAL_GET : `${BASE_URL}/admin/subapp/zyzx/artical/get`,  // 中医资讯获取文章内容
  SUBAPP_ZYZX_ARTICAL_UPDATE : `${BASE_URL}/admin/subapp/zyzx/artical/update`,  // 更新中医资讯获取文章
  SUBAPP_ZYZX_ARTICAL_DELETE : `${BASE_URL}/admin/subapp/zyzx/artical/delete`,  // 更新中医资讯获取文章


  COMMON_RESOURCES_ADD : `${BASE_URL}/admin/common/resources/add`,  // 上传资源
  COMMON_RESOURCES_DELETE : `${BASE_URL}/admin/common/resources/delete`,  // 删除资源
  COMMON_RESOURCES_GETLIST : `${BASE_URL}/admin/common/resources/getlist`,  // 删除资源


  SUBAPP_YSF_VIDEO_ADD : `${BASE_URL}/admin/subapp/ysf/video/add`,  // 养生坊添加视频
  SUBAPP_YSF_VIDEO_GET : `${BASE_URL}/admin/subapp/ysf/video/get`,  // 养生坊根据id获取信息
  SUBAPP_YSF_VIDEO_UPDATE : `${BASE_URL}/admin/subapp/ysf/video/update`,  // 养生坊更新视频信息
  SUBAPP_YSF_VIDEO_GETALL : `${BASE_URL}/admin/subapp/ysf/video/getall`,  // 养生坊获取全部视频
  SUBAPP_YSF_VIDEO_DELETE : `${BASE_URL}/admin/subapp/ysf/video/delete`,  // 养生坊删除视频

  SUBAPP_MJTYS_VIDEO_ADD : `${BASE_URL}/admin/subapp/mjtys/video/add`,  // 名家谈养生 添加视频
  SUBAPP_MJTYS_VIDEO_GET : `${BASE_URL}/admin/subapp/mjtys/video/get`,  // 名家谈养生根据id获取信息
  SUBAPP_MJTYS_VIDEO_UPDATE : `${BASE_URL}/admin/subapp/mjtys/video/update`,  // 名家谈养生更新视频信息
  SUBAPP_MJTYS_VIDEO_GETALL : `${BASE_URL}/admin/subapp/mjtys/video/getall`,  // 名家谈养生获取全部视频
  SUBAPP_MJTYS_VIDEO_DELETE : `${BASE_URL}/admin/subapp/mjtys/video/delete`,  // 名家谈养生删除视频

  SUBAPP_ZYWHKP_VIDEO_ADD : `${BASE_URL}/admin/subapp/zywhkp/video/add`,  // 中医文化科普 添加视频
  SUBAPP_ZYWHKP_VIDEO_GET : `${BASE_URL}/admin/subapp/zywhkp/video/get`,  // 中医文化科普 根据id获取信息
  SUBAPP_ZYWHKP_VIDEO_UPDATE : `${BASE_URL}/admin/subapp/zywhkp/video/update`,  // 中医文化科普 更新视频信息
  SUBAPP_ZYWHKP_VIDEO_GETALL : `${BASE_URL}/admin/subapp/zywhkp/video/getall`,  // 中医文化科普 获取全部视频
  SUBAPP_ZYWHKP_VIDEO_DELETE : `${BASE_URL}/admin/subapp/zywhkp/video/delete`,  // 中医文化科普 删除视频

}