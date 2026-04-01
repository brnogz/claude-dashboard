import SwiftUI

struct HeaderView: View {
    let stats: StatsResponse?
    let isConnected: Bool
    let onSettingsClick: () -> Void

    var body: some View {
        HStack {
            HStack(spacing: 6) {
                Text("CLAUDE")
                    .foregroundStyle(Color.claudeAmber)
                    .font(.system(size: 18, weight: .bold))
                Text("CODE")
                    .foregroundStyle(.white)
                    .font(.system(size: 18, weight: .bold))
            }
            Spacer()
            if let stats {
                Text("[\(stats.usage.plan.uppercased())]")
                    .foregroundStyle(Color.statusGreen)
                    .font(.system(size: 12))
                Text("\(stats.sessions.active) active")
                    .foregroundStyle(Color.textDim)
                    .font(.system(size: 12))
                    .padding(.leading, 4)
            }
            if !isConnected {
                Text("NO LINK")
                    .foregroundStyle(.red)
                    .font(.system(size: 10))
                    .padding(.leading, 4)
            }
            Button(action: onSettingsClick) {
                Image(systemName: "gearshape.fill")
                    .foregroundStyle(Color.claudeAmber)
                    .font(.system(size: 14))
            }
            .buttonStyle(.plain)
            .padding(.leading, 4)
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 8)
        .background(RoundedRectangle(cornerRadius: 12).fill(Color.cardBg))
    }
}
