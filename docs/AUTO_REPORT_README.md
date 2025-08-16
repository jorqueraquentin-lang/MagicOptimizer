# MagicOptimizer Auto-Report System

The MagicOptimizer plugin includes an automatic reporting system that sends usage data, error reports, and optimization results to a GitHub repository via a Cloudflare Worker endpoint. This system helps developers track issues, monitor plugin performance, and gather insights for continuous improvement.

## Overview

The auto-report system automatically collects and sends:
- **Error Reports**: When optimization phases fail or errors occur
- **Optimization Reports**: After each optimization phase completes
- **Session Reports**: At the end of plugin usage sessions
- **System Information**: UE version, platform, plugin version, and project details
- **Logs**: Recent plugin activity logs (configurable)
- **Knowledge Data**: Self-learning system insights and patterns

## Setup

### 1. Prerequisites

You need:
- A Cloudflare Worker endpoint configured to receive reports
- The Worker should be set up to create GitHub issues from the received data
- The Worker URL should be provided to you by your setup administrator

### 2. Configuration

Run the configuration utility to set up auto-reporting:

```bash
python tools/configure_auto_report.py
```

This interactive tool will:
- Ask if you want to enable auto-reporting
- Prompt for your Cloudflare Worker URL
- Configure data inclusion preferences
- Set privacy and reporting options
- Test the configuration

### 3. Manual Configuration

Alternatively, you can manually configure the system by editing the configuration file located at:
```
Saved/MagicOptimizer/Config/auto_report_config.json
```

Example configuration:
```json
{
  "enabled": true,
  "worker_url": "https://your-worker.your-subdomain.workers.dev/report",
  "include_logs": true,
  "include_knowledge": true,
  "max_log_lines": 1000,
  "report_errors": true,
  "report_optimizations": true,
  "report_sessions": false,
  "anonymize_data": true
}
```

## Configuration Options

### Basic Settings

- **`enabled`**: Enable/disable auto-reporting system
- **`worker_url`**: Your Cloudflare Worker endpoint URL

### Data Inclusion

- **`include_logs`**: Include recent plugin logs in reports
- **`include_knowledge`**: Include knowledge base data and insights
- **`max_log_lines`**: Maximum number of log lines to include

### Report Types

- **`report_errors`**: Send reports when errors occur
- **`report_optimizations`**: Send reports after optimization phases
- **`report_sessions`**: Send reports at session end

### Privacy

- **`anonymize_data`**: Anonymize identifying information in reports

## How It Works

1. **Data Collection**: The plugin automatically collects relevant information during operation
2. **Report Generation**: Reports are created with metadata, data, and optional attachments
3. **HTTP Transmission**: Reports are sent via POST to your Cloudflare Worker endpoint
4. **GitHub Integration**: The Worker creates GitHub issues with the received data
5. **Issue Tracking**: You can track and manage issues in your GitHub repository

## Report Types

### Error Reports

Sent when:
- Optimization phases fail
- Exceptions occur during processing
- Critical errors are encountered

Includes:
- Error type and message
- Context information
- System details
- Recent logs
- Knowledge base data

### Optimization Reports

Sent after:
- Audit phase completion
- Recommend phase completion
- Apply phase completion
- Verify phase completion

Includes:
- Phase results and statistics
- Assets processed and modified
- Processing duration
- Success/failure status
- System information

### Session Reports

Sent when:
- Plugin usage session ends
- User closes the plugin

Includes:
- Session summary
- Total assets processed
- Overall success status
- Session duration (if tracked)

## Privacy and Security

- **Local Processing**: All data is processed locally before transmission
- **Configurable Anonymization**: User data can be anonymized
- **Selective Inclusion**: Choose what data to include in reports
- **No External Dependencies**: Uses only built-in Python libraries
- **Secure Transmission**: Data is sent via HTTPS to your Worker endpoint

## Troubleshooting

### Common Issues

1. **Reports Not Sending**
   - Check if auto-reporting is enabled
   - Verify Worker URL is correct
   - Check network connectivity
   - Review plugin logs for error messages

2. **Configuration Not Saving**
   - Ensure write permissions to Saved directory
   - Check if UE is running in read-only mode
   - Verify configuration file path

3. **HTTP Errors**
   - Verify Worker endpoint is accessible
   - Check Worker URL format (must start with http:// or https://)
   - Ensure Worker is configured to accept POST requests

### Testing

Use the test script to verify your setup:

```bash
python tools/test_auto_report.py
```

This will test:
- Module imports
- Configuration loading
- System info gathering
- HTTP functionality
- Data collection

## Development

### Adding New Report Types

To add new report types, extend the `AutoReporter` class:

```python
def send_custom_report(self, data: dict) -> Tuple[bool, str, Optional[str]]:
    # Implementation for custom report type
    pass
```

### Customizing Data Collection

Modify the data gathering methods in `AutoReporter`:
- `_gather_system_info()`: System and environment information
- `_gather_logs()`: Plugin logs and activity
- `_gather_knowledge_data()`: Knowledge base insights

### Configuration Extensions

Add new configuration options in `AutoReportConfig`:
- Define new settings in `_get_default_config()`
- Add getter/setter methods
- Update configuration validation

## Support

If you encounter issues with the auto-report system:

1. Check the plugin logs for error messages
2. Verify your Cloudflare Worker configuration
3. Test with the provided test scripts
4. Review the configuration file for errors
5. Contact your setup administrator for Worker endpoint issues

## Future Enhancements

Planned improvements:
- Rate limiting and throttling
- Offline queue for failed reports
- Report compression and optimization
- Advanced privacy controls
- Custom report templates
- Integration with external monitoring systems
