import SwiftUI

struct PeriodCardView: View {
    let label: String
    let tokens: Int
    let messages: Int

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(label)
                .foregroundStyle(Color.textDim)
                .font(.system(size: 11))
            HStack(alignment: .bottom, spacing: 4) {
                Text(formatTokens(tokens))
                    .foregroundStyle(.white)
                    .font(.system(size: 22, weight: .bold))
                Text("tok")
                    .foregroundStyle(Color.textDim)
                    .font(.system(size: 12))
                    .padding(.bottom, 2)
            }
            Text("\(messages) msgs")
                .foregroundStyle(Color.textDim)
                .font(.system(size: 12))
        }
        .frame(maxWidth: .infinity, alignment: .leading)
        .padding(10)
        .background(RoundedRectangle(cornerRadius: 12).fill(Color.cardBg))
    }
}
