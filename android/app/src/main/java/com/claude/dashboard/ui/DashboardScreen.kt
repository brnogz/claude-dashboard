package com.claude.dashboard.ui

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Settings
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.claude.dashboard.DashboardUiState
import com.claude.dashboard.data.StatsResponse
import com.claude.dashboard.ui.theme.*

fun formatTokens(tokens: Long): String = when {
    tokens >= 1_000_000_000 -> "%.1fB".format(tokens / 1_000_000_000.0)
    tokens >= 1_000_000 -> "%.1fM".format(tokens / 1_000_000.0)
    tokens >= 1_000 -> "%.1fK".format(tokens / 1_000.0)
    else -> tokens.toString()
}

fun formatDuration(seconds: Long): String = when {
    seconds >= 3600 -> "%dh %02dm".format(seconds / 3600, (seconds % 3600) / 60)
    else -> "%dm".format(seconds / 60)
}

@Composable
fun DashboardScreen(state: DashboardUiState, onSettingsClick: () -> Unit = {}) {
    val stats = state.stats

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(MaterialTheme.colorScheme.background)
            .statusBarsPadding()
            .navigationBarsPadding()
            .padding(12.dp),
        verticalArrangement = Arrangement.spacedBy(10.dp),
    ) {
        // Header
        HeaderCard(stats, state.isConnected, onSettingsClick)

        if (stats == null) {
            if (state.error != null) {
                Text(
                    "Connecting to ${state.host}:${state.port}...",
                    color = TextDim,
                    modifier = Modifier.padding(16.dp)
                )
                Text(state.error, color = Color.Red, fontSize = 12.sp, modifier = Modifier.padding(horizontal = 16.dp))
            }
            return@Column
        }

        // Today / This Week
        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(10.dp)) {
            PeriodCard("TODAY", stats.today.totalTokens, stats.today.messages, Modifier.weight(1f))
            PeriodCard("THIS WEEK", stats.week.totalTokens, stats.week.messages, Modifier.weight(1f))
        }

        // Session
        SessionCard(stats)

        // Weekly chart — expands to fill remaining space
        WeeklyChart(stats.week.daily, stats.week.labels, Modifier.weight(1f))

        // Usage footer
        UsageCard(stats)
    }
}

@Composable
fun HeaderCard(stats: StatsResponse?, isConnected: Boolean, onSettingsClick: () -> Unit = {}) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(containerColor = CardBg),
        shape = RoundedCornerShape(12.dp),
    ) {
        Row(
            modifier = Modifier.fillMaxWidth().padding(start = 16.dp, top = 8.dp, bottom = 8.dp, end = 4.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically,
        ) {
            Row {
                Text("CLAUDE", color = ClaudeAmber, fontSize = 22.sp, fontWeight = FontWeight.Bold)
                Spacer(Modifier.width(6.dp))
                Text("CODE", color = Color.White, fontSize = 22.sp, fontWeight = FontWeight.Bold)
            }
            Row(verticalAlignment = Alignment.CenterVertically) {
                if (stats != null) {
                    Text("[${stats.usage.plan.uppercase()}]", color = Color(0xFF00E676), fontSize = 14.sp)
                    Spacer(Modifier.width(8.dp))
                    Text("${stats.sessions.active} active", color = TextDim, fontSize = 14.sp)
                }
                if (!isConnected) {
                    Spacer(Modifier.width(8.dp))
                    Text("NO LINK", color = Color.Red, fontSize = 12.sp)
                }
                IconButton(onClick = onSettingsClick) {
                    Icon(
                        Icons.Default.Settings,
                        contentDescription = "Settings",
                        tint = ClaudeAmber,
                    )
                }
            }
        }
    }
}

@Composable
fun PeriodCard(label: String, tokens: Long, messages: Int, modifier: Modifier) {
    Card(
        modifier = modifier,
        colors = CardDefaults.cardColors(containerColor = CardBg),
        shape = RoundedCornerShape(12.dp),
    ) {
        Column(modifier = Modifier.padding(12.dp)) {
            Text(label, color = TextDim, fontSize = 12.sp)
            Spacer(Modifier.height(4.dp))
            Row(verticalAlignment = Alignment.Bottom) {
                Text(formatTokens(tokens), color = Color.White, fontSize = 28.sp, fontWeight = FontWeight.Bold)
                Spacer(Modifier.width(4.dp))
                Text("tok", color = TextDim, fontSize = 14.sp, modifier = Modifier.padding(bottom = 4.dp))
            }
            Spacer(Modifier.height(2.dp))
            Text("$messages msgs", color = TextDim, fontSize = 14.sp)
        }
    }
}

@Composable
fun SessionCard(stats: StatsResponse) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(containerColor = CardBg),
        shape = RoundedCornerShape(12.dp),
    ) {
        Column(modifier = Modifier.padding(12.dp)) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
            ) {
                Text("SESSION: ${stats.current.id}", color = ClaudeAmber, fontSize = 14.sp)
                Text(formatDuration(stats.current.duration), color = TextDim, fontSize = 14.sp)
            }
            Spacer(Modifier.height(4.dp))
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
            ) {
                Text("Tokens: ${formatTokens(stats.current.totalTokens)}", color = Color.White, fontSize = 14.sp)
                Text("Msgs: ${stats.current.messages}", color = TextDim, fontSize = 14.sp)
            }
        }
    }
}

@Composable
fun WeeklyChart(daily: List<Long>, labels: List<String>, modifier: Modifier = Modifier) {
    Card(
        modifier = modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(containerColor = CardBg),
        shape = RoundedCornerShape(12.dp),
    ) {
        Column(modifier = Modifier.fillMaxSize().padding(12.dp)) {
            Text("WEEKLY", color = TextDim, fontSize = 12.sp)
            Spacer(Modifier.height(8.dp))

            val maxVal = daily.maxOrNull()?.coerceAtLeast(1L) ?: 1L

            Canvas(
                modifier = Modifier.fillMaxWidth().weight(1f).defaultMinSize(minHeight = 80.dp)
            ) {
                val barCount = daily.size.coerceAtMost(7)
                val spacing = size.width / barCount
                val barWidth = spacing * 0.6f
                val labelHeight = 20f

                for (i in 0 until barCount) {
                    val barH = if (daily[i] > 0) {
                        ((daily[i].toFloat() / maxVal) * (size.height - labelHeight)).coerceAtLeast(4f)
                    } else 0f

                    val x = i * spacing + (spacing - barWidth) / 2
                    val y = size.height - labelHeight - barH

                    val color = if (i == barCount - 1) ClaudeAmber else ClaudeCyan

                    if (barH > 0) {
                        drawRect(
                            color = color,
                            topLeft = Offset(x, y),
                            size = Size(barWidth, barH),
                        )
                    }
                }
            }

            // Day labels
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceAround,
            ) {
                labels.take(7).forEach { label ->
                    Text(label, color = TextDim, fontSize = 11.sp, modifier = Modifier.width(30.dp))
                }
            }
        }
    }
}

@Composable
fun UsageCard(stats: StatsResponse) {
    val pct = stats.usage.pct
    val barColor = when {
        pct > 80 -> Color.Red
        pct > 50 -> Color.Yellow
        else -> Color(0xFF00E676)
    }

    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(containerColor = CardBg),
        shape = RoundedCornerShape(12.dp),
    ) {
        Column(modifier = Modifier.padding(12.dp)) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
            ) {
                Text("${stats.usage.used}/${stats.usage.limit} msgs", color = Color.White, fontSize = 16.sp)
                Text("(${stats.usage.window})", color = TextDim, fontSize = 14.sp)
            }
            Spacer(Modifier.height(8.dp))
            LinearProgressIndicator(
                progress = { pct / 100f },
                modifier = Modifier.fillMaxWidth().height(10.dp),
                color = barColor,
                trackColor = Color(0xFF333333),
            )
            Spacer(Modifier.height(4.dp))
            Text("${100 - pct}% remaining", color = TextDim, fontSize = 12.sp)
        }
    }
}
