<script setup lang="ts">
import { defineVaDataTableColumns, useModal } from 'vuestic-ui'
import { Node } from '../types'
import { PropType, computed, toRef } from 'vue'
import { Pagination, Sorting } from '../../../data/pages/nodes'
import { useVModel } from '@vueuse/core'
import { Project } from '../../projects/types'

const columns = defineVaDataTableColumns([
  { label: 'ID', key: 'id', sortable: true },
  { label: 'Type', key: 'type', sortable: true },
  { label: 'Name', key: 'name', sortable: true },
  { label: 'Update time', key: 'update_time', sortable: true },
  { label: 'Workload', key: 'workload', sortable: true },
  { label: 'Public IP', key: 'public_ip', sortable: true },
  { label: 'Port', key: 'port', sortable: true },
  { label: 'Status', key: 'status', sortable: true },
  { label: 'NetConnection', key: 'connections', sortable: true },
  { label: 'NetClientConnection', key: 'net_client_connections', sortable: true },
  { label: ' ', key: 'actions', align: 'right' },
])

const props = defineProps({
  users: {
    type: Array as PropType<Node[]>,
    required: true,
  },
  loading: { type: Boolean, default: false },
  pagination: { type: Object as PropType<Pagination>, required: true },
  sortBy: { type: String as PropType<Sorting['sortBy']>, required: true },
  sortingOrder: { type: String as PropType<Sorting['sortingOrder']>, required: true },
})

const emit = defineEmits<{
  (event: 'edit-user', user: Node): void
  (event: 'delete-user', user: Node): void
  (event: 'update:sortBy', sortBy: Sorting['sortBy']): void
  (event: 'update:sortingOrder', sortingOrder: Sorting['sortingOrder']): void
}>()

const users = toRef(props, 'users')
const sortByVModel = useVModel(props, 'sortBy', emit)
const sortingOrderVModel = useVModel(props, 'sortingOrder', emit)

const totalPages = computed(() => Math.ceil(props.pagination.total / props.pagination.perPage))

const { confirm } = useModal()

const onUserDelete = async (user: Node) => {
  const agreed = await confirm({
    title: 'Delete user',
    message: `Are you sure you want to delete ${user.name}?`,
    okText: 'Delete',
    cancelText: 'Cancel',
    size: 'small',
    maxWidth: '380px',
  })

  if (agreed) {
    emit('delete-user', user)
  }
}

const formatProjectNames = (projects: Project[]) => {
  if (projects == null || projects.length === 0) return 'No projects'
  if (projects.length <= 2) {
    return projects.map((project) => project.project_name).join(', ')
  }

  return (
    projects
      .slice(0, 2)
      .map((project) => project.project_name)
      .join(', ') +
    ' + ' +
    (projects.length - 2) +
    ' more'
  )
}
</script>

<template>
  <VaDataTable
    v-model:sort-by="sortByVModel"
    v-model:sorting-order="sortingOrderVModel"
    :columns="columns"
    :items="users"
    :loading="$props.loading"
  >

    <template #cell(name)="{ rowData }">
      <div class="max-w-[120px] ellipsis">
        {{ rowData.name }}
      </div>
    </template>

    <template #cell(actions)="{ rowData }">
      <div class="flex gap-2 justify-end">
        <VaButton
          preset="primary"
          size="small"
          icon="mso-edit"
          aria-label="Edit user"
          @click="$emit('edit-user', rowData as Node)"
        />
        <VaButton
          preset="primary"
          size="small"
          icon="mso-delete"
          color="danger"
          aria-label="Delete user"
          @click="onUserDelete(rowData as Node)"
        />
      </div>
    </template>
  </VaDataTable>

  <div class="flex flex-col-reverse md:flex-row gap-2 justify-between items-center py-2">
    <div>
      <b>{{ $props.pagination.total }} results.</b>
      Results per page
      <VaSelect v-model="$props.pagination.perPage" class="!w-20" :options="[10, 50, 100]" />
    </div>

    <div v-if="totalPages > 1" class="flex">
      <VaButton
        preset="secondary"
        icon="va-arrow-left"
        aria-label="Previous page"
        :disabled="$props.pagination.page === 1"
        @click="$props.pagination.page--"
      />
      <VaButton
        class="mr-2"
        preset="secondary"
        icon="va-arrow-right"
        aria-label="Next page"
        :disabled="$props.pagination.page === totalPages"
        @click="$props.pagination.page++"
      />
      <VaPagination
        v-model="$props.pagination.page"
        buttons-preset="secondary"
        :pages="totalPages"
        :visible-pages="5"
        :boundary-links="false"
        :direction-links="false"
      />
    </div>
  </div>
</template>

<style lang="scss" scoped>
.va-data-table {
  ::v-deep(.va-data-table__table-tr) {
    border-bottom: 1px solid var(--va-background-border);
  }
}
</style>
