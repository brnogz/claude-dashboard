import SwiftUI

struct UsageFooterView: View {
    let usage: UsageData

    private var barColor: Color {
        switch usage.pct {
        case 81...: .red
        case 51...80: .yellow
        default: .statusGreen
        }
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            HStack {
                Text("\(usage.used)/\(usage.limit) msgs")
                    .foregroundStyle(.white)
                    .font(.system(size: 14))
                Spacer()
                Text("(\(usage.window))")
                    .foregroundStyle(Color.textDim)
                    .font(.system(size: 12))
            }

            GeometryReader { geo in
                ZStack(alignment: .leading) {
                    RoundedRectangle(cornerRadius: 4)
                        .fill(Color(white: 0.2))
                        .frame(height: 8)
                    RoundedRectangle(cornerRadius: 4)
                        .fill(barColor)
                        .frame(width: geo.size.width * CGFloat(usage.pct) / 100.0, height: 8)
                }
            }
            .frame(height: 8)

            Text("\(100 - usage.pct)% remaining")
                .foregroundStyle(Color.textDim)
                .font(.system(size: 11))
        }
        .padding(10)
        .background(RoundedRectangle(cornerRadius: 12).fill(Color.cardBg))
    }
}
