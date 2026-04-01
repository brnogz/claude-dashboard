package com.claude.dashboard.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.claude.dashboard.ui.theme.*

@Composable
fun SettingsScreen(
    currentHost: String,
    currentPort: Int,
    onSave: (String, Int) -> Unit,
    onDismiss: () -> Unit,
) {
    var host by remember { mutableStateOf(currentHost) }
    var port by remember { mutableStateOf(currentPort.toString()) }

    Column(
        modifier = Modifier
            .fillMaxWidth()
            .background(MaterialTheme.colorScheme.background)
            .padding(16.dp)
    ) {
        Text("Settings", color = ClaudeAmber, fontSize = 20.sp)
        Spacer(Modifier.height(16.dp))

        OutlinedTextField(
            value = host,
            onValueChange = { host = it },
            label = { Text("Host IP") },
            modifier = Modifier.fillMaxWidth(),
            singleLine = true,
            colors = OutlinedTextFieldDefaults.colors(
                focusedBorderColor = ClaudeAmber,
                unfocusedBorderColor = TextDim,
                focusedLabelColor = ClaudeAmber,
                cursorColor = ClaudeAmber,
            ),
        )
        Spacer(Modifier.height(8.dp))

        OutlinedTextField(
            value = port,
            onValueChange = { port = it },
            label = { Text("Port") },
            modifier = Modifier.fillMaxWidth(),
            singleLine = true,
            keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number),
            colors = OutlinedTextFieldDefaults.colors(
                focusedBorderColor = ClaudeAmber,
                unfocusedBorderColor = TextDim,
                focusedLabelColor = ClaudeAmber,
                cursorColor = ClaudeAmber,
            ),
        )
        Spacer(Modifier.height(16.dp))

        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            Button(
                onClick = { onSave(host, port.toIntOrNull() ?: 8080) },
                colors = ButtonDefaults.buttonColors(containerColor = ClaudeAmber),
                shape = RoundedCornerShape(8.dp),
            ) {
                Text("Save", color = Color.Black)
            }
            OutlinedButton(
                onClick = onDismiss,
                shape = RoundedCornerShape(8.dp),
            ) {
                Text("Cancel", color = TextDim)
            }
        }
    }
}
