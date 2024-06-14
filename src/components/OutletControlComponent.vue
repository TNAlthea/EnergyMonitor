<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import mqtt from 'mqtt'

const props = defineProps({
  client: {
    type: mqtt.MqttClient,
    required: true
  }
})

const deviceStatuses = ref({} as Record<string, { status: string; lastUpdate: Date }>)
const relayStatuses = ref({} as Record<string, { relay_1: string; relay_2: string }>)

onMounted(() => {
  if (props.client) {
    props.client.on('message', handleMqttMessage)
    subscribeToTopics()
  }
})

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
}

const handleDeviceStatusMessage = (topic: string, message: Buffer) => {
  try {
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

const extractDeviceId = (topic: string) => {
  return topic.split('/')[4] // Extract device ID from topic
}

const subscribeToTopics = () => {
  props.client.subscribe('2024/2005021/esp32/device_status/#')
  props.client.subscribe('2024/2005021/esp32/relay/#')
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
</script>

<template>
  <div class="py-5">
    <h2 class="text-lg font-semibold">Status</h2>
    <div>
      <div v-for="(deviceStatus, deviceId) in deviceStatuses" :key="deviceId">
        <p>Device ID: {{ deviceId }}</p>
        <p>Status: {{ deviceStatus.status }}</p>
        <div class="flex flex-row gap-5">
          <section v-if="relayStatuses[deviceId]">
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
          </section>
          <section v-else>
            <p>Waiting for Relay Status (ETA 2 seconds)</p>
          </section>
        </div>
        <p>Last Update: {{ deviceStatus.lastUpdate }}</p>
        <hr />
      </div>
    </div>
  </div>
</template>
