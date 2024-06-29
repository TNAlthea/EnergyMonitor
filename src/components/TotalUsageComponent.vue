<script setup lang="ts">
import { ref, onMounted, computed } from 'vue'
import mqtt from 'mqtt'
import axios from 'axios'
import { ArrowPathIcon, ArrowsRightLeftIcon } from '@heroicons/vue/24/outline'

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

const powerUsage = ref()
const TotalPowerFormatPerHour = ref(true) // true = W, false = kWh

const getPowerData = async () => {
  try {
    // Construct the URL with the parameter
    const baseUrl = import.meta.env.VITE_GET_TOTAL_POWER_DATA_MONTH
    const url = baseUrl.replace(':month', requestData.month)
    // DATA FOR CURRENT MONTH
    const response = await axios.get(url)

    powerUsage.value = response.data.data
  } catch (error) {
    console.error(error)
  }
}

const formatTotalPower = computed(() => {
  return Number(powerUsage.value[0].total_power).toFixed(2) + ' W'
})

const formatKWH = computed(() => {
  return Number(powerUsage.value[0].total_power / 1000).toFixed(2) + ' kWh'
})

const togglePowerFormat = () => {
  TotalPowerFormatPerHour.value = !TotalPowerFormatPerHour.value
}


onMounted(() => {
  getPowerData()
})
</script>

<template>
  <div class="relative w-full rounded-xl bg-white bg-opacity-80 p-5">
    <div class="flex flex-col gap-2">
      <p class="text">Total Penggunaan Daya Bulan Ini</p>
      <div class="rounded w-full bg-teal-50 p-5">
        <div v-if="powerUsage" class=" flex flex-row gap-5">
          <span>
            <p v-if="!TotalPowerFormatPerHour" class="text-2xl">{{ formatTotalPower }}</p>
            <p v-else class="text-2xl">{{ formatKWH }}</p>
          </span>
          <span class="">
            <button @click="togglePowerFormat"  class="flex flex-row items-end group">
              <ArrowsRightLeftIcon class="w-4 h-4 text-gray-500 group-hover:text-black" />
              <p class="text-sm text-gray-500 group-hover:text-black group-hover:font-semibold">Toggle mode</p>
            </button>
          </span>
        </div>
        <div v-else>
          <p>Sorry, data can't be fetched right now. Please try again later.</p>
        </div>
      </div>
    </div>
    <div class="absolute right-2.5 top-2.5">
      <div class="flex flex-row gap-1 justify-center items-center group" @click="getPowerData">
        <ArrowPathIcon
          class="w-5 h-5 text-black group-hover:text-gray-700 group-hover:cursor-pointer"
        />
        <p class="group-hover:text-gray-700 group-hover:cursor-pointer">Refresh Data</p>
      </div>
    </div>
  </div>
</template>
