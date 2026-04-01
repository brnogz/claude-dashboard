import SwiftUI

struct SessionCardView: View {
    let session: CurrentSession

    var body: some View {
        VStack(spacing: 4) {
            HStack {
                Text("SESSION: \(session.id)")
                    .foregroundStyle(Color.claudeAmber)
                    .font(.system(size: 14))
                Spacer()
                Text(formatDuration(session.duration))
                    .foregroundStyle(Color.textDim)
                    .font(.system(size: 14))
            }
            HStack {
                Text("Tokens: \(formatTokens(session.totalTokens))")
                    .foregroundStyle(.white)
                    .font(.system(size: 14))
                Spacer()
                Text("Msgs: \(session.messages)")
                    .foregroundStyle(Color.textDim)
                    .font(.system(size: 14))
            }
        }
        .padding(12)
        .background(RoundedRectangle(cornerRadius: 12).fill(Color.cardBg))
    }
}
