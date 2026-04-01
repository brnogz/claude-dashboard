package com.claude.dashboard

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.lifecycle.viewmodel.compose.viewModel
import com.claude.dashboard.ui.DashboardScreen
import com.claude.dashboard.ui.SettingsScreen
import com.claude.dashboard.ui.theme.ClaudeDashboardTheme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            ClaudeDashboardTheme {
                val vm: DashboardViewModel = viewModel()
                val state by vm.state.collectAsStateWithLifecycle()
                var showSettings by remember { mutableStateOf(false) }

                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background,
                ) {
                    if (showSettings) {
                        SettingsScreen(
                            currentHost = state.host,
                            currentPort = state.port,
                            onSave = { host, port ->
                                vm.updateHost(host, port)
                                showSettings = false
                            },
                            onDismiss = { showSettings = false },
                        )
                    } else {
                        DashboardScreen(
                            state = state,
                            onSettingsClick = { showSettings = true },
                        )
                    }
                }
            }
        }
    }
}
