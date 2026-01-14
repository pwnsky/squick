import { sleep } from '../../services/utils'
import { User } from './../../pages/users/types'
import { httpAsyncPost, httpAsyncGet } from '../../net/http';


// Simulate API calls
export type Pagination = {
  page: number
  perPage: number
  total: number
}

export type Sorting = {
  sortBy: keyof Node | undefined
  sortingOrder: 'asc' | 'desc' | null
}

export type Filters = {
  isActive: boolean
  search: string
}

const getSortItem = (obj: any, sortBy: string) => {
  if (sortBy === 'projects') {
    return obj.projects.map((project: any) => project.project_name).join(', ')
  }

  return obj[sortBy]
}

export const getNodes = async (filters: Partial<Filters & Pagination & Sorting>) => {
  await sleep(500)
  try {
    var data = await httpAsyncGet("/api/all_nodes")
  } catch (error) {
    console.error('Error fetching node data:', error);
    return {
        data: {},
        pagination: {
          page: 0,
          perPage: 0,
          total: 0,
        },
      }
  }
  
  

  const { isActive, search, sortBy, sortingOrder } = filters
  let filteredUsers = data.data.node_list
  console.log("nodes: ", filteredUsers)

  if (search) {
    filteredUsers = filteredUsers.filter((user) => user.fullname.toLowerCase().includes(search.toLowerCase()))
  }

  if (sortBy && sortingOrder) {
    filteredUsers = filteredUsers.sort((a, b) => {
      const first = getSortItem(a, sortBy)
      const second = getSortItem(b, sortBy)
      if (first > second) {
        return sortingOrder === 'asc' ? 1 : -1
      }
      if (first < second) {
        return sortingOrder === 'asc' ? -1 : 1
      }
      return 0
    })
  }

  const { page = 1, perPage = 10 } = filters || {}
  return {
    data: filteredUsers.slice((page - 1) * perPage, page * perPage),
    pagination: {
      page,
      perPage,
      total: filteredUsers.length,
    },
  }
}

export const addUser = async (user: Node) => {
  await sleep(1000)
  //users.unshift(user)
}

export const updateUser = async (user: Node) => {
  await sleep(1000)
  //const index = users.findIndex((u) => u.id === user.id)
  //users[index] = user
}

export const removeUser = async (user: Node) => {
  await sleep(1000)
  //users.splice(
  //  users.findIndex((u) => u.id === user.id),
  //  1,
  //)
}
