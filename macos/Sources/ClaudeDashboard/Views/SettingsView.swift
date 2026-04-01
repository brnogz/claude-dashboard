import SwiftUI

struct SettingsView: View {
    @Bindable var settings: SettingsManager
    let onDismiss: () -> Void

    @State private var hostText: String = ""
    @State private var portText: String = ""

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("Settings")
                .foregroundStyle(Color.claudeAmber)
                .font(.system(size: 18, weight: .bold))

            VStack(alignment: .leading, spacing: 4) {
                Text("Host").foregroundStyle(Color.textDim).font(.system(size: 12))
                TextField("Host", text: $hostText)
                    .textFieldStyle(.roundedBorder)
            }
            VStack(alignment: .leading, spacing: 4) {
                Text("Port").foregroundStyle(Color.textDim).font(.system(size: 12))
                TextField("Port", text: $portText)
                    .textFieldStyle(.roundedBorder)
            }

            HStack(spacing: 8) {
                Button("Save") {
                    settings.host = hostText
                    settings.port = Int(portText) ?? 8080
                    onDismiss()
                }
                .buttonStyle(.borderedProminent)
                .tint(.claudeAmber)

                Button("Cancel", action: onDismiss)
                    .buttonStyle(.bordered)
            }
            .padding(.top, 4)
        }
        .padding(16)
        .frame(width: 280)
        .background(Color.darkBg)
        .onAppear {
            hostText = settings.host
            portText = "\(settings.port)"
        }
    }
}
