package com.claude.dashboard.ui.theme

import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color

val ClaudeAmber = Color(0xFFFFA500)
val ClaudeCyan = Color(0xFF00FFFF)
val DarkBg = Color(0xFF0A0A0A)
val CardBg = Color(0xFF1A1A2E)
val TextDim = Color(0xFFB0B0B0)

private val DarkScheme = darkColorScheme(
    primary = ClaudeAmber,
    secondary = ClaudeCyan,
    background = DarkBg,
    surface = CardBg,
    onPrimary = Color.Black,
    onSecondary = Color.Black,
    onBackground = Color.White,
    onSurface = Color.White,
)

@Composable
fun ClaudeDashboardTheme(content: @Composable () -> Unit) {
    MaterialTheme(
        colorScheme = DarkScheme,
        content = content,
    )
}
