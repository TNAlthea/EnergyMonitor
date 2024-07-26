<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { useRoute } from 'vue-router';
import {
  PresentationChartBarIcon,
  PresentationChartLineIcon,
  Cog6ToothIcon,
  EyeIcon
} from '@heroicons/vue/24/outline'

const emit = defineEmits(['current-menu'])

const route = useRoute() // Step 2: Use useRoute to get the current route object
const pathSegment = route.path.split('/').pop() // Step 3: Get the last segment of the path

const mode = ref(pathSegment || 'Dashboard')
const changeMenuMode = (item: string) => {
  item = item.replace(/\s/g, "")
  mode.value = item
  emit('current-menu', item)
}

onMounted(() => {
  emit('current-menu', 'Dashboard')
})

const removeSpaces = (string: string) => {
  return string.replace(/\s/g, '')
}

const menuItems = ref([
  {
    icon: PresentationChartLineIcon,
    text: 'Dashboard',
    handler: changeMenuMode
  },
  {
    icon: PresentationChartBarIcon,
    text: 'Monitor',
    handler: changeMenuMode
  },
  // {
  //   icon: Cog6ToothIcon,
  //   text: 'Settings',
  //   handler: changeMenuMode
  // },
  {
      icon: EyeIcon,
      text: 'Anomaly Log',
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
      <router-link :to="item.text.replace(/\s/g, '')" v-for="(item, index) in menuItems" :key="index" @click="item.handler(item.text)">
        <div v-if="mode == removeSpaces(item.text)" class="flex flex-row gap-3 group">
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
