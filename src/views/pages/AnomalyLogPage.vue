<script setup lang="ts">
import { ref, onMounted } from 'vue'
import axios from 'axios'
import mqtt from 'mqtt'

import OutletAnomalyStatsComponent from '@/components/OutletAnomalyStatsComponent.vue'
import OutletAnomalyLogDetailsComponent from '@/components/OutletAnomalyLogDetailsComponent.vue'

const props = defineProps({
  anomalyData: {
    type: Array as () => {
      id: string
      data_id: string
      device_id: string
      if_labels_anomaly: string
      rf_labels_anomaly: string
      current: number
      voltage: number
      power: number
      energy: number
      frequency: number
      power_factor: number
      created_at: string
    }[],
    required: true
  }
})

const showDetailedStats = ref(false)
const receiveAnomalyDataByDeviceId = (id: string) => {
  showDetailedStats.value = true
  getElectricalDataLogs(id)
}

const electricalDataLogs = ref([])
const getElectricalDataLogs = async (id: string) => {
  try {
    const baseUrl = import.meta.env.VITE_GET_ELECTRICITY_ANOMALY_DATA_BY_DEVICE_ID
    const url = baseUrl.replace(':device_id', id)
    const response = await axios.get(url)
    electricalDataLogs.value = response.data.data

  } catch (error) {
    console.error(error)
  }
}

onMounted(() => {
  showDetailedStats.value = false
})
</script>

<template>
  <div v-if="props.anomalyData">
    <OutletAnomalyStatsComponent
      v-if="!showDetailedStats"
      :anomalyData="props.anomalyData"
      @current-id="receiveAnomalyDataByDeviceId"
    />
    <OutletAnomalyLogDetailsComponent v-else :electricalDataLogs="electricalDataLogs" />
  </div>
  <div v-else>
    <p>Loading data...</p>
  </div>
</template>
