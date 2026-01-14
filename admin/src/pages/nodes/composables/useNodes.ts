import { Ref, ref, unref, watch } from 'vue'
import { getNodes, updateUser, addUser, removeUser, type Filters, Pagination, Sorting } from '../../../data/pages/nodes'
import { User } from '../types'
import { watchIgnorable } from '@vueuse/core'

const makePaginationRef = () => ref<Pagination>({ page: 1, perPage: 10, total: 0 })
const makeSortingRef = () => ref<Sorting>({ sortBy: 'name', sortingOrder: null })
const makeFiltersRef = () => ref<Partial<Filters>>({ isActive: true, search: '' })

export const useUsers = (options?: {
  pagination?: Ref<Pagination>
  sorting?: Ref<Sorting>
  filters?: Ref<Partial<Filters>>
}) => {
  const isLoading = ref(false)
  const users = ref<Node[]>([])

  const { filters = makeFiltersRef(), sorting = makeSortingRef(), pagination = makePaginationRef() } = options || {}

  const fetch = async () => {
    isLoading.value = true
    const { data, pagination: newPagination } = await getNodes({
      ...unref(filters),
      ...unref(sorting),
      ...unref(pagination),
    })
    users.value = data

    ignoreUpdates(() => {
      pagination.value = newPagination
    })

    isLoading.value = false
  }

  const { ignoreUpdates } = watchIgnorable([pagination, sorting], fetch, { deep: true })

  watch(
    filters,
    () => {
      // Reset pagination to first page when filters changed
      pagination.value.page = 1
      fetch()
    },
    { deep: true },
  )

  fetch()

  return {
    isLoading,

    filters,
    sorting,
    pagination,

    users,

    fetch,

    async add(user: Node) {
      isLoading.value = true
      await addUser(user)
      await fetch()
      isLoading.value = false
    },

    async update(user: Node) {
      isLoading.value = true
      await updateUser(user)
      await fetch()
      isLoading.value = false
    },

    async remove(user: Node) {
      isLoading.value = true
      await removeUser(user)
      await fetch()
      isLoading.value = false
    },
  }
}
