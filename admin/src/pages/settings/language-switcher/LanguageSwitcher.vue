<template>
  <div class="flex items-center justify-between">
    <p>Language</p>
    <div class="w-40">
      <VaSelect v-model="model" :options="options" />
    </div>
  </div>
</template>
<script lang="ts" setup>
import { computed } from 'vue'

import { useI18n } from 'vue-i18n'

type LanguageMap = Record<string, string>

const { locale } = useI18n()

const languages: LanguageMap = {
  english: 'English',
  simplified_chinese: 'Simplified Chinese',
}

const languageCodes: LanguageMap = {
  gb: languages.english,
  cn: languages.simplified_chinese,
}

const languageName: LanguageMap = Object.fromEntries(Object.entries(languageCodes).map(([key, value]) => [value, key]))

const options = Object.values(languageCodes)

const model = computed({
  get() {
    return languageCodes[locale.value]
  },
  set(value) {
    locale.value = languageName[value]
  },
})
</script>
