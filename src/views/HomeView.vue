<script setup lang="ts">
import axios from 'axios'
import { onMounted, onUnmounted, ref } from 'vue'
import mqtt from 'mqtt'
import SidebarComponent from '@/components/SidebarComponent.vue'
import { RouterView } from 'vue-router'
import { eventBus } from '../utils/eventBus.js' // Adjust the path as necessary

interface ElectricalData {
  voltage: number
  current: number
  power: number
  energy: number
  frequency: number
  power_factor: number
}

interface LastUpdateTimes {
  timestamp: Date
}

type lastUpdateTimeMap = Record<string, LastUpdateTimes>
type ElectricalDataMap = Record<string, ElectricalData>

const routerViewRef = ref()
const electricalData = ref<ElectricalDataMap>({})
const lastUpdate = ref<lastUpdateTimeMap>({})

let client: mqtt.MqttClient
const connected = ref(false)

const electricalDataTopic = '2024/2005021/esp32/telemetry/'
let intervalId: NodeJS.Timeout | null = null

onMounted(() => {
  try {
    client = mqtt.connect('ws://broker.emqx.io:8083/mqtt')
    client.on('connect', () => {
      connected.value = true
    })

    if (client) {
      client.subscribe(`${electricalDataTopic}#`)
      client.on('message', handleMqttMessage)
    }

    intervalId = setInterval(resetElectricalData, 3000)

    getAnomalyLog()
  } catch (error) {
    console.error(error)
  }
})

onUnmounted(() => {
  if (intervalId) {
    clearInterval(intervalId)
  }
})

const mode = ref('Dashboard')
const receiveSidebarEmit = (menu: string) => {
  mode.value = menu
}

const extractDeviceId = (topic: string) => {
  return topic.split('/')[4] // Extract device ID from topic
}

const handleMqttMessage = (topic: string, message: Buffer) => {
  if (topic.startsWith(electricalDataTopic)) {
    handleElectricalDataMessage(topic, message)
  }
}

const handleElectricalDataMessage = (topic: string, message: Buffer) => {
  try {
    const deviceId = extractDeviceId(topic)
    const msg = JSON.parse(message.toString())

    // Update electricalData with received data
    electricalData.value[deviceId] = {
      voltage: msg.voltage,
      current: msg.current,
      power: msg.power,
      energy: msg.energy,
      frequency: msg.frequency,
      power_factor: msg.power_factor
    }

    lastUpdate.value[deviceId] = {
      timestamp: new Date()
    }

    // Calculate total electrical data
    callCalculateTotalElectricalData()
  } catch (error) {
    console.error(error)
  }
}

// Function to reset values to zero for devices that haven't updated in the last 10 seconds
const resetElectricalData = () => {
  const now = Date.now()
  for (const key in electricalData.value) {
    const time = Number(lastUpdate.value[key].timestamp)
    if (now - time >= 10000) {
      electricalData.value[key] = {
        current: 0,
        voltage: 0,
        power: 0,
        energy: 0,
        frequency: 0,
        power_factor: 0
      }
      callCalculateTotalElectricalData()
      // delete electricalData.value[key]
      // delete lastUpdate.value[key]
    }
  }
}

const callCalculateTotalElectricalData = () => {
  eventBus.triggerCalculateTotalElectricalData = true
}

const anomalyData = ref([])
const getAnomalyLog = async () => {
    try {
        const response = await axios.get(import.meta.env.VITE_GET_ELECTRICITY_ANOMALY_DATA)   
        anomalyData.value = response.data.data
    } catch (error) {
        console.error(error)
    }
}
</script>

<template>
  <div class="flex flex-row h-screen max-h-full max-w-full w-screen">
    <nav>
      <SidebarComponent @current-menu="receiveSidebarEmit" />
    </nav>
    <main class="w-full bg-gradient-to-tl from-blue-600 via-blue-700 to-violet-800 p-10 overflow-scroll">
      <div v-if="connected">
        <RouterView
          :client="client"
          :electricalData="electricalData"
          :lastUpdate="lastUpdate"
          :anomalyData="anomalyData"
          ref="routerViewRef"
        />
      </div>
      <div v-else>
        <p>Waiting for MQTT Client (Connected Value: {{ connected }})</p>
      </div>
    </main>
  </div>
</template>
