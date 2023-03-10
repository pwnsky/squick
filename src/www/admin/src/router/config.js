import TabsView from '@/layouts/tabs/TabsView'
import BlankView from '@/layouts/BlankView'
import PageView from '@/layouts/PageView'

// 路由配置
const options = {
  routes: [
    {
      path: '/login',
      name: '登录页',
      component: () => import('@/pages/login')
    },
    {
      path: '*',
      name: '404',
      component: () => import('@/pages/exception/404'),
    },
    {
      path: '/403',
      name: '403',
      component: () => import('@/pages/exception/403'),
    },
    {
      path: '/',
      name: '首页',
      component: TabsView,
      redirect: '/login',
      children: [
        {
          path: 'dashboard',
          name: '统计',
          meta: {
            icon: 'dashboard'

          },
          component: BlankView,
          children: [
            {
              path: 'workplace',
              name: '统计分析',
              component: () => import('@/pages/dashboard/devices/Index.vue'),
            },
          ]
        },

         

        {
          path: 'sub_app',
          name: '子程序',
          meta: {
            icon: 'form',
            page: {
              cacheAble: false
            }
          },
          component: BlankView,
          children: [
            {
              path: 'sztzsb',
              name: '舌诊体质AI识别',
              component: BlankView,
              children: [
                {
                  path: 'view',
                  name: '查看最近图片',
                  component: () => import('@/pages/subapp/sztzsb/view/View'),
                },
              ]
            },
            {
              path: 'zyzx',
              name: '中医资讯',
              component: BlankView,
              children: [
                {
                  path: 'new',
                  name: '新增文章',
                  component: () => import('@/pages/subapp/zyzx/add/View'),
                },
                {
                  path: 'list',
                  name: '文章列表',
                  component: () => import('@/pages/subapp/zyzx/list/View'),
                },
                
              ]
              
            },
            {
              path: 'zywhkp',
              name: '中医文化科普',
              component: BlankView,
              children: [
                {
                  path: 'add',
                  name: '添加视频',
                  component: () => import('@/pages/subapp/zywhkp/add/View'),
                },
                {
                  path: 'list',
                  name: '视频列表',
                  component: () => import('@/pages/subapp/zywhkp/list/View'),
                },
              ]
            },
            {
              path: 'syxxl',
              name: '识药消消乐',
              
            },
            {
              path: 'dzhgj',
              name: '大综合歌诀',
              
            },
            {
              path: 'zyfjx',
              name: '中医方剂学',
              
            },
          ]
        },


        {
          path: 'users',
          name: '用户',
          meta: {
            icon: 'form',
            page: {
              cacheAble: false
            }
          },
          component: PageView,
          children: [
            {
              path: 'all',
              name: '已注册用户',
              component: BlankView,
            },
            {
              path: 'log',
              name: '用户日志',
              component: BlankView,
            },
          ]
        },


        
        {
          path: 'common',
          name: '公共',
          meta: {
            icon: 'form',
            page: {
              cacheAble: false
            }
          },
          component: PageView,
          children: [
            {
              path: 'gateway',
              name: '网关设置',
              component: BlankView,
            },
            {
              path: 'list',
              name: '已上传资源列表',
              component: BlankView,
            },
            {
              path: 'step',
              name: '版本控制',

            },
          ]
        },

      


        


        /*
        {
          path: 'form',
          name: '表单页',
          meta: {
            icon: 'form',
            page: {
              cacheAble: false
            }
          },
          component: PageView,
          children: [
            {
              path: 'basic',
              name: '基础表单',
              component: () => import('@/pages/form/basic'),
            },
            {
              path: 'step',
              name: '分步表单',
              component: () => import('@/pages/form/step'),
            },
            {
              path: 'advance',
              name: '高级表单',
              component: () => import('@/pages/form/advance'),
            }
          ]
        },

        {
          path: 'list',
          name: '列表页',
          meta: {
            icon: 'table'
          },
          component: PageView,
          children: [
            {
              path: 'query',
              name: '查询表格',
              meta: {
                authority: 'queryForm',
              },
              component: () => import('@/pages/list/QueryList'),
            },
            {
              path: 'query/detail/:id',
              name: '查询详情',
              meta: {
                highlight: '/list/query',
                invisible: true
              },
              component: () => import('@/pages/subapp/Demo')
            },
            {
              path: 'primary',
              name: '标准列表',
              component: () => import('@/pages/list/StandardList'),
            },
            {
              path: 'card',
              name: '卡片列表',
              component: () => import('@/pages/list/CardList'),
            },
            {
              path: 'search',
              name: '搜索列表',
              component: () => import('@/pages/list/search/SearchLayout'),
              children: [
                {
                  path: 'article',
                  name: '文章',
                  component: () => import('@/pages/list/search/ArticleList'),
                },
                {
                  path: 'application',
                  name: '应用',
                  component: () => import('@/pages/list/search/ApplicationList'),
                },
                {
                  path: 'project',
                  name: '项目',
                  component: () => import('@/pages/list/search/ProjectList'),
                }
              ]
            }
          ]
        },
        {
          path: 'details',
          name: '详情页',
          meta: {
            icon: 'profile'
          },
          component: BlankView,
          children: [
            {
              path: 'basic',
              name: '基础详情页',
              component: () => import('@/pages/detail/BasicDetail')
            },
            {
              path: 'advance',
              name: '高级详情页',
              component: () => import('@/pages/detail/AdvancedDetail')
            }
          ]
        },
        {
          path: 'result',
          name: '结果页',
          meta: {
            icon: 'check-circle-o',
          },
          component: PageView,
          children: [
            {
              path: 'success',
              name: '成功',
              component: () => import('@/pages/result/Success')
            },
            {
              path: 'error',
              name: '失败',
              component: () => import('@/pages/result/Error')
            }
          ]
        },
        {
          path: 'exception',
          name: '异常页',
          meta: {
            icon: 'warning',
          },
          component: BlankView,
          children: [
            {
              path: '404',
              name: 'Exp404',
              component: () => import('@/pages/exception/404')
            },
            {
              path: '403',
              name: 'Exp403',
              component: () => import('@/pages/exception/403')
            },
            {
              path: '500',
              name: 'Exp500',
              component: () => import('@/pages/exception/500')
            }
          ]
        },
        {
          path: 'components',
          name: '内置组件',
          meta: {
            icon: 'appstore-o'
          },
          component: PageView,
          children: [
            {
              path: 'taskCard',
              name: '任务卡片',
              component: () => import('@/pages/components/TaskCard')
            },
            {
              path: 'palette',
              name: '颜色复选框',
              component: () => import('@/pages/components/Palette')
            },
            {
              path: 'table',
              name: '高级表格',
              component: () => import('@/pages/components/table')
            }
          ]
        },
        {
          name: '验权表单',
          path: 'auth/form',
          meta: {
            icon: 'file-excel',
            authority: {
              permission: 'form'
            }
          },
          component: () => import('@/pages/form/basic')
        },
        {
          name: '带参菜单',
          path: 'router/query',
          meta: {
            icon: 'project',
            query: {
              name: '菜单默认参数'
            }
          },
          component: () => import('@/pages/subapp/Demo')
        },
        {
          name: '动态路由菜单',
          path: 'router/dynamic/:id',
          meta: {
            icon: 'project',
            params: {
              id: 123
            }
          },
          component: () => import('@/pages/subapp/Demo')
        },
        {
          name: 'Ant Design Vue',
          path: 'antdv',
          meta: {
            icon: 'ant-design',
            link: 'https://www.antdv.com/docs/vue/introduce-cn/'
          }
        },
        {
          name: '使用文档',
          path: 'document',
          meta: {
            icon: 'file-word',
            link: 'https://iczer.gitee.io/vue-antd-admin-docs/'
          }
        }*/
      ]
    },
    
  ]
}

export default options
