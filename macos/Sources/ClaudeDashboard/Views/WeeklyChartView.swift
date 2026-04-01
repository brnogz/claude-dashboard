import SwiftUI

struct WeeklyChartView: View {
    let daily: [Int]
    let labels: [String]

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("WEEKLY")
                .foregroundStyle(Color.textDim)
                .font(.system(size: 11))

            GeometryReader { geo in
                let barCount = min(daily.count, 7)
                let maxVal = max(daily.max() ?? 1, 1)
                let spacing = geo.size.width / CGFloat(barCount)
                let barWidth = spacing * 0.6

                Canvas { context, size in
                    for i in 0..<barCount {
                        let barH: CGFloat = daily[i] > 0
                            ? max(CGFloat(daily[i]) / CGFloat(maxVal) * size.height, 4)
                            : 0
                        let x = CGFloat(i) * spacing + (spacing - barWidth) / 2
                        let y = size.height - barH
                        let color: Color = (i == barCount - 1) ? .claudeAmber : .claudeCyan

                        if barH > 0 {
                            context.fill(
                                Path(CGRect(x: x, y: y, width: barWidth, height: barH)),
                                with: .color(color)
                            )
                        }
                    }
                }
            }
            .frame(minHeight: 80)

            HStack(spacing: 0) {
                ForEach(Array(labels.prefix(7).enumerated()), id: \.offset) { _, label in
                    Text(label)
                        .foregroundStyle(Color.textDim)
                        .font(.system(size: 10))
                        .frame(maxWidth: .infinity)
                }
            }
        }
        .padding(10)
        .background(RoundedRectangle(cornerRadius: 12).fill(Color.cardBg))
    }
}
