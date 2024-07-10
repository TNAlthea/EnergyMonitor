<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import mqtt from 'mqtt'

interface ElectricalData {
  voltage: number
  current: number
  power: number
  energy: number
  frequency: number
  power_factor: number
  [key: string]: any
}

interface ElectricalDataMap {
  [key: string]: ElectricalData
}

const props = defineProps({
  client: {
    type: mqtt.MqttClient,
    required: true
  },
  electricalData: {
    type: Object as () => ElectricalDataMap,
    required: true
  }
})
const isLoading = ref(true)
const deviceStatuses = ref({} as Record<string, { status: string; lastUpdate: Date }>)
const relayStatuses = ref({} as Record<string, { relay_1: string; relay_2: string }>)
const powerLimit = ref(0)
const powerStatus = ref({})

onMounted(() => {
  if (props.client) {
    props.client.on('message', handleMqttMessage)
    subscribeToTopics()
  }
})

const subscribeToTopics = () => {
  props.client.subscribe('2024/2005021/esp32/device_status/#')
  props.client.subscribe('2024/2005021/esp32/relay/#')
  props.client.subscribe('2024/2005021/esp32/config/power/#')
}

const resetDeviceStatuses = () => {
  deviceStatuses.value = {}
}
const handleMqttMessage = (topic: string, message: Buffer) => {
  if (topic.startsWith('2024/2005021/esp32/device_status/')) {
    handleDeviceStatusMessage(topic, message)
  }

  if (topic.startsWith('2024/2005021/esp32/relay/')) {
    handleRelayMessage(topic, message)
  }

  if (topic.startsWith('2024/2005021/esp32/config/power/')) {
    handlePowerConfigMessage(topic, message)
  }
}

const handleDeviceStatusMessage = (topic: string, message: Buffer) => {
  try {
    isLoading.value = false
    const deviceId = extractDeviceId(topic)
    deviceStatuses.value[deviceId] = {
      status: message.toString(),
      lastUpdate: new Date()
    }

    askRelayStatus(deviceId)
  } catch (error) {
    console.error(error)
  }
}

const handleRelayMessage = (topic: string, message: Buffer) => {
  try {
    const deviceId = extractDeviceId(topic)
    relayStatuses.value[deviceId] = JSON.parse(message.toString())
  } catch (error) {
    console.error(error)
  }
}

const handlePowerConfigMessage = (topic: string, message: Buffer) => {
  try {
    const deviceId = extractDeviceId(topic)

    const powerMessage = JSON.parse(message.toString())
    powerStatus.value[deviceId] = powerMessage
  } catch (error) {
    console.error(error)
  }
}

const extractDeviceId = (topic: string) => {
  return topic.split('/')[4] // Extract device ID from topic
}

const askRelayStatus = async (clientID: string) => {
  try {
    const message = {
      message: 'relay?'
    }
    props.client.publish(`2024/2005021/esp32/server/${clientID}`, JSON.stringify(message))
  } catch (error) {
    console.error(error)
  }
}

const toggleRelay = (deviceId: string, relay: string, currentStatus: string) => {
  const otherRelay = relay === 'relay_1' ? 'relay_2' : 'relay_1'
  try {
    if (props.client && relayStatuses.value[deviceId]) {
      const message = {
        [relay]: currentStatus === 'active' ? 'inactive' : 'active',
        [otherRelay]: relayStatuses.value[deviceId][otherRelay]
      }
      props.client.publish(`2024/2005021/esp32/relay/${deviceId}`, JSON.stringify(message))
    }
  } catch (error) {
    console.error(error)
  }
}

const setPowerLimit = (deviceId: string) => {
  try {
    if (props.client && powerLimit.value) {
      const message = {
        power_limit: powerLimit.value
      }
      props.client.publish(`2024/2005021/esp32/config/power/${deviceId}`, JSON.stringify(message))
    }
  } catch (error) {
    console.error(error)
  }
}

const formattedValue = (value: number) => {
  return value.toFixed(2)
}
</script>

<template>
  <div class="p-5 bg-white bg-opacity-80 rounded-lg overflow-scroll">
    <h2 class="text-lg font-semibold pb-5">Device Status</h2>
    <section v-if="isLoading == true">Waiting for data...</section>
    <section v-else>
      <div
        v-for="(deviceStatus, deviceId) in deviceStatuses"
        :key="deviceId"
        class="bg-teal-50 p-5 rounded-lg"
      >
        <div class="flex flex-col">
          <div>
            <section v-if="relayStatuses[deviceId]">
              <div class="grid grid-flow-cols grid-cols-2 gap-5">
                <section class="" id="deviceStatus">
                  <p>Device ID: {{ deviceId }}</p>
                  <p>Status: {{ deviceStatus.status }}</p>
                  <div class="flex flex-row gap-5">
                    <p>Relay 1: {{ relayStatuses[deviceId].relay_1 }}</p>
                    <button
                      class="hover:text-blue-500"
                      @click="toggleRelay(deviceId, 'relay_1', relayStatuses[deviceId].relay_1)"
                    >
                      Toggle Relay 1
                    </button>
                  </div>

                  <div class="flex flex-row gap-5">
                    <p>Relay 2: {{ relayStatuses[deviceId].relay_2 }}</p>
                    <button
                      class="hover:text-blue-500"
                      @click="toggleRelay(deviceId, 'relay_2', relayStatuses[deviceId].relay_2)"
                    >
                      Toggle Relay 2
                    </button>
                  </div>
                  <span>
                    <p>Last Update: {{ deviceStatus.lastUpdate }}</p>
                  </span>
                  <div class="my-3 p-3 bg-teal-100 rounded">
                    <div>
                      <label>Power Limit</label>
                      <div class="flex flex-row items-center justify-between gap-3">
                        <span class="flex flex-row w-32 gap-2">
                          <input class="px-3 w-full" type="number" v-model="powerLimit" :min="0" />
                          <p>W</p>
                        </span>
                        <button
                          class="bg-blue-300 hover:bg-blue-500 px-3 py-1"
                          @click="setPowerLimit(deviceId)"
                        >
                          <p>Set</p>
                        </button>
                      </div>
                    </div>
                  </div>
                  <div v-if="powerStatus.power.status === 'overload'" class="my-3 p-3 bg-red-500">
                    <p class="text-xl font-bold text-white">OVERLOAD</p>
                  </div>
                </section>
                <section class="grid grid-cols-3 gap-3" id="electricalData">
                  <div class="px-1 border-2 border-gray-500 relative overflow-hidden">
                    <p>Current:</p>
                    <p class="text-2xl">
                      {{ formattedValue(props.electricalData[deviceId].current) + ' Ampere' }}
                    </p>
                    <p
                      class="absolute bottom-0 right-0 translate-y-1.5 translate-x-1 font-semibold text-6xl text-gray-500 opacity-20"
                    >
                      A
                    </p>
                  </div>
                  <div class="px-1 border-2 border-gray-500 relative overflow-hidden">
                    <p>Voltage:</p>
                    <p class="text-2xl">
                      {{ formattedValue(props.electricalData[deviceId].voltage) + ' Volt' }}
                    </p>
                    <p
                      class="absolute bottom-0 right-0 translate-y-1.5 translate-x-1 font-semibold text-6xl text-gray-500 opacity-20"
                    >
                      V
                    </p>
                  </div>
                  <div class="px-1 border-2 border-gray-500 relative overflow-hidden">
                    <p>Power:</p>
                    <p class="text-2xl">
                      {{ formattedValue(props.electricalData[deviceId].power) + ' Watt' }}
                    </p>
                    <p
                      class="absolute bottom-0 right-0 translate-y-1.5 translate-x-1 font-semibold text-6xl text-gray-500 opacity-20"
                    >
                      W
                    </p>
                  </div>
                  <div class="px-1 border-2 border-gray-500 relative overflow-hidden">
                    <p>Energy:</p>
                    <p class="text-2xl">
                      {{ formattedValue(props.electricalData[deviceId].energy) + ' Joule' }}
                    </p>
                    <p
                      class="absolute bottom-0 right-0 translate-y-1.5 font-semibold text-6xl text-gray-500 opacity-20"
                    >
                      J
                    </p>
                  </div>
                  <div class="px-1 border-2 border-gray-500 relative overflow-hidden">
                    <p class="text-sm">Power Factor:</p>
                    <p class="text-2xl">
                      {{ formattedValue(props.electricalData[deviceId].power_factor) + ' p.f' }}
                    </p>
                    <p
                      class="absolute bottom-0 right-0 translate-y-0.5 translate-x-0.5 font-semibold text-6xl text-gray-500 opacity-20"
                    >
                      p.f
                    </p>
                  </div>
                  <div class="px-1 border-2 border-gray-500 relative overflow-hidden">
                    <p>Frequency:</p>
                    <p class="text-2xl">
                      {{ formattedValue(props.electricalData[deviceId].frequency) + ' Hz' }}
                    </p>
                    <p
                      class="absolute bottom-0 right-0 translate-y-1.5 translate-x-1 font-semibold text-6xl text-gray-500 opacity-20"
                    >
                      Hz
                    </p>
                  </div>
                </section>
              </div>
            </section>
            <section v-else>
              <p>Waiting for Relay Status (ETA 2 seconds)</p>
            </section>
          </div>
        </div>
      </div>
    </section>
  </div>
</template>
