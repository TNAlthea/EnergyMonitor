<script setup lang="ts">
import { ref, onMounted } from 'vue'
import {
  PresentationChartBarIcon,
  PresentationChartLineIcon,
  Cog6ToothIcon
} from '@heroicons/vue/24/outline'

const emit = defineEmits(['current-menu'])

const mode = ref('dashboard')
const changeMenuMode = (item: string) => {
  mode.value = item
  emit('current-menu', item)
}

const menuItems = ref([
  {
    icon: PresentationChartLineIcon,
    text: 'dashboard',
    handler: changeMenuMode
  },
  {
    icon: PresentationChartBarIcon,
    text: 'monitor',
    handler: changeMenuMode
  },
  {
    icon: Cog6ToothIcon,
    text: 'settings',
    handler: changeMenuMode
  }
])
</script>
<template>
  <main class="h-full min-w-[15vw] p-10 bg-blue-950 flex flex-col">
    <span class="flex flex-row gap-3 justify-center border-b border-white pb-5">
      <p class="text-4xl font-bold text-white">Smergy</p>
      <p class="text-blue-100">Alpha</p>
    </span>
    <section id="menuItems" class="py-10 flex flex-col gap-10 justify-center items-start">
      <router-link :to="item.text" v-for="(item, index) in menuItems" :key="index" @click="item.handler(item.text)">
        <div v-if="mode == item.text" class="flex flex-row gap-3 group">
          <component :is="item.icon" class="w-10 text-sky-300 group-hover:text-sky-300"></component>
          <p class="top-0.5 text-xl relative text-sky-300 group-hover:text-sky-300">
            {{ item.text }}
          </p>
        </div>
        <div v-else class="flex flex-row gap-3 group">
          <component :is="item.icon" class="w-10 text-sky-700 group-hover:text-sky-300"></component>
          <p class="top-0.5 text-xl relative text-sky-700 group-hover:text-sky-300">
            {{ item.text }}
          </p>
        </div>
      </router-link>
    </section>
  </main>
</template>
