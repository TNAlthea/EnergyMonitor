<script setup lang="ts">
import axios from 'axios'
import mqtt from 'mqtt'
import { ref, onMounted, onUnmounted, computed } from 'vue'

const props = defineProps({
  client: {
    type: mqtt.MqttClient,
    required: true
  }
})

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
type ElectricalDataMap = Record<string, ElectricalData> // Interface for source -> data mapping

const electricalData = ref<ElectricalDataMap>({}) // Initialize as empty object

const totalElectricalData = ref<ElectricalData>({
  voltage: 0,
  current: 0,
  power: 0,
  energy: 0,
  frequency: 0,
  power_factor: 0
})

const lastUpdate = ref<lastUpdateTimeMap>({})

const electricalDataTopic = '2024/2005021/esp32/telemetry/'

let intervalId: NodeJS.Timeout | null = null
onMounted(async () => {
  intervalId = setInterval(resetElectricalData, 3000)
  if (props.client) {
    props.client.subscribe(`${electricalDataTopic}#`)
    props.client.on('message', handleMqttMessage)
  }
})

onUnmounted(() => {
  if (intervalId) {
    clearInterval(intervalId)
  }
})

const handleMqttMessage = (topic: string, message: Buffer) => {
  if (topic.startsWith(electricalDataTopic)) {
    console.log('Received message from topic: ', topic)
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

    // Store electrical data
    storeElectricalData({ ...electricalData.value[deviceId] })

    // Calculate total electrical data
    calculateTotalElectricalData()
  } catch (error) {
    console.error(error)
  }
}

const extractDeviceId = (topic: string) => {
  return topic.split('/')[4] // Extract device ID from topic
}

// Function to reset values to zero for devices that haven't updated in the last 10 seconds
const resetElectricalData = () => {
  const now = Date.now()
  for (const key in electricalData.value) {
    const time = Number(lastUpdate.value[key].timestamp)
    if (now - time >= 10000) {
      console.log('delete data for device because of inactivity: ', key)
      electricalData.value[key] = {
        current: 0,
        voltage: 0,
        power: 0,
        energy: 0,
        frequency: 0,
        power_factor: 0
      }
      calculateTotalElectricalData()
      delete electricalData.value[key]
      delete lastUpdate.value[key]
    }
  }
}

const storeElectricalData = async (data: ElectricalData) => {
  console.log('called: ', new Date())

  try {
    const response = await axios.post(import.meta.env.VITE_STORE_ELECTRICITY_DATA, data)
  } catch (error) {
    console.error(error)
  }
}

const calculateTotalElectricalData = () => {
  resetTotalElectricalData()
  for (const key in electricalData.value) {
    const data = electricalData.value[key]
    totalElectricalData.value.voltage += data.voltage
    totalElectricalData.value.current += data.current
    totalElectricalData.value.power += data.power
    totalElectricalData.value.energy += data.energy
    totalElectricalData.value.frequency += data.frequency
    totalElectricalData.value.power_factor += data.power_factor
  }
}

const resetTotalElectricalData = () => {
  totalElectricalData.value = {
    voltage: 0,
    current: 0,
    power: 0,
    energy: 0,
    frequency: 0,
    power_factor: 0
  }
}

const adjustedTotalElectricalDataFormat = computed(() => {
  return {
    voltage: Number(totalElectricalData.value.voltage).toFixed(2),
    current: Number(totalElectricalData.value.current).toFixed(2),
    power: Number(totalElectricalData.value.power).toFixed(2),
    energy: Number(totalElectricalData.value.energy).toFixed(2),
    frequency: Number(totalElectricalData.value.frequency).toFixed(2),
    power_factor: Number(totalElectricalData.value.power_factor).toFixed(2)
  }
})
</script>
<template>
  <section
    v-if="electricalData"
    class="grid grid-cols-3 grid-flow-col gap-4 p-5 bg-cyan-50 bg-opacity-80 rounded-md"
    id="totalData"
  >
    <div class="relative p-3 rounded-md bg-teal-50 overflow-hidden">
      <div class="flex flex-col">
        <p>Current:</p>
        <p class="text-2xl font-semibold">{{ adjustedTotalElectricalDataFormat.current }}</p>
      </div>
      <span class="absolute -bottom-2 -right-1">
        <p class="text-7xl font-semibold opacity-50">A</p>
      </span>
    </div>
    <div class="relative p-3 rounded-md bg-teal-50 overflow-hidden">
      <div class="flex flex-col">
        <p>Voltage:</p>
        <p class="text-2xl font-semibold">{{ adjustedTotalElectricalDataFormat.voltage }}</p>
      </div>
      <span class="absolute -bottom-2 -right-1">
        <p class="text-7xl font-semibold opacity-50">V</p>
      </span>
    </div>
    <div class="relative p-3 rounded-md bg-teal-50 overflow-hidden">
      <div class="flex flex-col">
        <p>Power:</p>
        <p class="text-2xl font-semibold">{{ adjustedTotalElectricalDataFormat.power }}</p>
      </div>
      <span class="absolute -bottom-2 -right-1">
        <p class="text-7xl font-semibold opacity-50">W</p>
      </span>
    </div>
  </section>
  <section v-else>
    <p>No data available</p>
  </section>
</template>
