import { createRouter, createWebHistory } from 'vue-router'
import HomeView from '../views/HomeView.vue'

import DashboardPage from '@/views/pages/DashboardPage.vue'
import MonitorPage from '@/views/pages/MonitorPage.vue'
import SettingsPage from '@/views/pages/SettingsPage.vue'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    {
      path: '/',
      name: 'home',
      component: HomeView,
      redirect: '/dashboard',
      children: [
        {
          path: 'dashboard',
          component: DashboardPage
        },
        {
          path: 'monitor',
          component: MonitorPage
        },
        {
          path: 'settings',
          component: SettingsPage 
        }
      ]
    },
  ]
})

export default router
