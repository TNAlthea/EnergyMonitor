<script setup lang="ts">
import { onMounted, ref } from 'vue'
import mqtt from 'mqtt'
import SidebarComponent from '@/components/SidebarComponent.vue'

let client: mqtt.MqttClient
const connected = ref(false)
onMounted(() => {
  try {
    client = mqtt.connect('ws://broker.emqx.io:8083/mqtt')
    client.on('connect', () => {
      connected.value = true
    })
  } catch (error) {
    console.error(error)
  }
})
const mode = ref('Dashboard')
const receiveSidebarEmit = (menu: string) => {
  mode.value = menu
}
</script>

<template>
  <div class="flex flex-row h-screen max-h-full max-w-full w-screen">
    <nav>
      <SidebarComponent @current-menu="receiveSidebarEmit" />
    </nav>
    <main class="w-full bg-gradient-to-tl from-blue-600 via-blue-700 to-violet-800 p-10">
      <div v-if="connected">
        <RouterView :client="client"/>
      </div>
      <div v-else>
        <p>Waiting for MQTT Client (Connected Value: {{ connected }})</p>
      </div>
    </main>
  </div>
</template>
