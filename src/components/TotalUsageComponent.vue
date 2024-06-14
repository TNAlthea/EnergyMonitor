<script setup lang="ts">
import { ref, onMounted, computed } from 'vue'
import mqtt from 'mqtt'
import axios from 'axios'
import { ArrowPathIcon } from '@heroicons/vue/24/outline'

const props = defineProps({
  client: {
    type: mqtt.MqttClient,
    required: true
  }
})
const dateNow: Date = new Date()
const requestData = {
  month: dateNow.getMonth() + 1
}

const currentUsage = ref()

const getCurrentData = async () => {
  try {
    // Construct the URL with the parameter
    const baseUrl = import.meta.env.VITE_GET_TOTAL_CURRENT_DATA_MONTH
    const url = baseUrl.replace(':month', requestData.month)
    // DATA FOR CURRENT MONTH
    const response = await axios.get(url)

    currentUsage.value = response.data.data
  } catch (error) {
    console.error(error)
  }
}

const formatTotalCurrent = computed(() => {
  return Number(currentUsage.value[0].total_current).toFixed(2) + ' A'
})

onMounted(() => {
  getCurrentData()
})
</script>

<template>
  <div class="relative w-full rounded-xl bg-white bg-opacity-80 p-5">
    <div class="flex flex-col gap-2">
      <p class="text">Total Penggunan Arus Bulan Ini</p>
      <div class="rounded w-full bg-teal-50 p-5">
        <p v-if="currentUsage" class="text-2xl">{{ formatTotalCurrent }}</p>
        <p v-else>Sorry, data can't be fetched right now. Please try again later.</p>
      </div>
    </div>
    <div class="absolute right-2.5 top-2.5">
      <div class="flex flex-row gap-1 justify-center items-center group" @click="getCurrentData">
        <ArrowPathIcon
          class="w-5 h-5 text-black group-hover:text-gray-700 group-hover:cursor-pointer"
        />
        <p class="group-hover:text-gray-700 group-hover:cursor-pointer">Refresh Data</p>
      </div>
    </div>
  </div>
</template>
