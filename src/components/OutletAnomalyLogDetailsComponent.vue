<script setup lang="ts">
import { ref, onMounted, computed, watch } from 'vue'

const props = defineProps({
  electricalDataLogs: {
    type: Object,
    required: true
  }
})

onMounted(() => {})

const currentPage = ref(1)
const pageSize = 30

const paginatedLogs = computed(() => {
  const startIndex = (currentPage.value - 1) * pageSize
  const endIndex = startIndex + pageSize
  return props.electricalDataLogs.slice(startIndex, endIndex)
})

const filteredLogs = computed(() => {
  const startIndex = (currentPage.value - 1) * pageSize
  const endIndex = startIndex + pageSize
  return props.electricalDataLogs
    .filter((item) => item.rf_labels_anomaly === 1)
    .slice(startIndex, endIndex)
})

watch(filteredLogs, (newVal) => {
  console.log('filteredLogs length:', newVal.length)
})

const totalPages = computed(() => {
  return Math.ceil(props.electricalDataLogs.length / pageSize)
})

// const totalPages = computed(() => {
//   return Math.ceil(props.electricalDataLogs.length / pageSize)
// })

const prevPage = () => {
  if (currentPage.value > 1) {
    currentPage.value--
  } else {
    currentPage.value = totalPages.value
  }
}

const nextPage = () => {
  if (currentPage.value < totalPages.value) {
    currentPage.value++
  } else {
    currentPage.value = 1
  }
}

const formattedDate = (date: string) => {
  return new Date(date).toLocaleString('en-US', {
    year: 'numeric',
    month: 'long',
    day: 'numeric',
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit'
  })
}

const formattedValue = (value: number) => {
  return value.toFixed(2)
}

const formattedAnomaly = (value: number) => {
  return value === 1
    ? {
        value: 'Anomaly',
        class: 'bg-red-500'
      }
    : {
        value: 'Normal',
        class: 'bg-green-500'
      }
}
</script>

<template>
  <div v-if="electricalDataLogs">
    <span class="px-5 text-white">
      <h1 class="text-2xl font-bold text-center">Anomaly Log Details</h1>
      <!-- <h2 class="text-center">For Device ID: {{ paginatedLogs[0].device_id }}</h2> -->
    </span>
    <section class="bg-cyan-100 opacity-80 p-5 rounded-md max-h-[700px]">
      <!-- Scrollable container for the table -->
      <div class="overflow-x-auto overflow-y-auto max-h-[600px]">
        <table class="table-fixed w-full">
          <thead>
            <tr>
              <th class="">Current</th>
              <th class="">Voltage</th>
              <th class="">Power</th>
              <th class="">Energy</th>
              <th class="">Frequency</th>
              <th class="">Power Factor</th>
              <th class="">Behaviour</th>
              <th class="">Recorded At</th>
            </tr>
          </thead>
          <tbody class="">
            <tr
              v-for="(item, index) in filteredLogs"
              :key="index"
              class="border-b border-slate-300 bg-white"
            >
              <td class="text-center">{{ formattedValue(item.current) }}</td>
              <td class="text-center">{{ formattedValue(item.voltage) }}</td>
              <td class="text-center">{{ formattedValue(item.power) }}</td>
              <td class="text-center">{{ formattedValue(item.energy) }}</td>
              <td class="text-center">{{ formattedValue(item.frequency) }}</td>
              <td class="text-center">{{ formattedValue(item.power_factor) }}</td>
              <!-- <td class="text-center" :class="formattedAnomaly(item.if_labels_anomaly).class">
                {{ formattedAnomaly(item.if_labels_anomaly).value }}
              </td> -->
              <td class="text-center" :class="formattedAnomaly(item.rf_labels_anomaly).class">
                {{ formattedAnomaly(item.rf_labels_anomaly).value }}
              </td>
              <td class="text-end px-3">{{ formattedDate(item.created_at) }}</td>
            </tr>
          </tbody>
        </table>
      </div>
    </section>

    <div class="flex justify-center mt-4">
      <button class="px-4 py-2 mx-2 text-white bg-blue-500 rounded-md" @click="prevPage">
        Prev
      </button>
      <button class="px-4 py-2 mx-2 text-white bg-blue-500 rounded-md" @click="nextPage">
        Next
      </button>
    </div>
  </div>
  <div v-else>
    <p>Loading data...</p>
  </div>
</template>
