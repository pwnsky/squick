<script setup lang="ts">
import { ref } from 'vue'
import NodesTable from './widgets/NodesTable.vue'
import { User } from './types'
import { useUsers } from './composables/useNodes'
import { useModal, useToast } from 'vuestic-ui'

const doShowEditUserModal = ref(false)

const { users, isLoading, filters, sorting, pagination, ...usersApi } = useUsers()

const userToEdit = ref<User | null>(null)


const { init: notify } = useToast()

const onUserDelete = async (user: User) => {
  await usersApi.remove(user)
  notify({
    message: `${user.fullname} has been deleted`,
    color: 'success',
  })
}

const editFormRef = ref()

const { confirm } = useModal()
</script>

<template>
  <h1 class="page-title">{{ $t('menu.nodes') }}</h1>

  <VaCard>
    <VaCardContent>
      <div class="flex flex-col md:flex-row gap-2 mb-2 justify-between">
        <div class="flex flex-col md:flex-row gap-2 justify-start">
          <VaInput v-model="filters.search" placeholder="Search">
            <template #prependInner>
              <VaIcon name="search" color="secondary" size="small" />
            </template>
          </VaInput>
        </div>
      </div>

      <NodesTable
        v-model:sort-by="sorting.sortBy"
        v-model:sorting-order="sorting.sortingOrder"
        :users="users"
        :loading="isLoading"
        :pagination="pagination"
        @editUser="showEditUserModal"
        @deleteUser="onUserDelete"
      />
    </VaCardContent>
  </VaCard>
</template>
./composables/useNodes