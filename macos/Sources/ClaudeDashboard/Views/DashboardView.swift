import SwiftUI

struct DashboardView: View {
    @Bindable var viewModel: DashboardViewModel

    var body: some View {
        VStack(spacing: 10) {
            HeaderView(stats: viewModel.stats, isConnected: viewModel.isConnected) {
                viewModel.showSettings.toggle()
            }

            if let stats = viewModel.stats {
                HStack(spacing: 10) {
                    PeriodCardView(label: "TODAY", tokens: stats.today.totalTokens, messages: stats.today.messages)
                    PeriodCardView(label: "THIS WEEK", tokens: stats.week.totalTokens, messages: stats.week.messages)
                }

                SessionCardView(session: stats.current)
                WeeklyChartView(daily: stats.week.daily, labels: stats.week.labels)
                UsageFooterView(usage: stats.usage)
            } else if let error = viewModel.error {
                Spacer()
                Text("Connecting to \(viewModel.settings.host):\(viewModel.settings.port)...")
                    .foregroundStyle(Color.textDim)
                    .padding()
                Text(error)
                    .foregroundStyle(.red)
                    .font(.caption)
                    .padding(.horizontal)
                Spacer()
            } else {
                Spacer()
            }
        }
        .padding(12)
        .frame(width: 320, height: 480)
        .background(Color.darkBg)
        .sheet(isPresented: $viewModel.showSettings) {
            SettingsView(settings: viewModel.settings) {
                viewModel.showSettings = false
                viewModel.restartPolling()
            }
        }
    }
}
