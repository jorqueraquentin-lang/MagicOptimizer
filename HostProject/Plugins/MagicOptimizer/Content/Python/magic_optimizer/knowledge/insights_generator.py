"""
Insights Generator for MagicOptimizer plugin.

Generates human-readable markdown reports from analyzed knowledge data
to provide actionable insights for plugin improvement.
"""

import os
import json
from datetime import datetime
from typing import Dict, List, Any
from .pattern_analyzer import PatternAnalyzer


class InsightsGenerator:
    """Generates human-readable insights from knowledge data."""
    
    def __init__(self, knowledge_dir: str):
        """Initialize the insights generator.
        
        Args:
            knowledge_dir: Path to the knowledge directory
        """
        self.knowledge_dir = knowledge_dir
        self.analyzer = PatternAnalyzer(knowledge_dir)
        
    def generate_knowledge_summary(self) -> str:
        """Generate a comprehensive knowledge summary markdown report.
        
        Returns:
            Path to the generated summary file
        """
        summary_file = os.path.join(self.knowledge_dir, "knowledge_summary.md")
        
        # Get analysis data
        patterns = self.analyzer.analyze_usage_patterns()
        
        # Generate markdown content
        content = self._generate_summary_content(patterns)
        
        # Write to file
        with open(summary_file, 'w', encoding='utf-8') as f:
            f.write(content)
        
        return summary_file
    
    def _generate_summary_content(self, patterns: Dict[str, Any]) -> str:
        """Generate markdown content for the summary.
        
        Args:
            patterns: Analysis patterns dictionary
            
        Returns:
            Markdown content string
        """
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S UTC')
        
        content = f"""# MagicOptimizer Knowledge Summary

*Generated on {timestamp}*

## 📊 Executive Summary

This report summarizes usage patterns, optimization insights, and recommendations
based on data collected from MagicOptimizer plugin usage across multiple projects.

## 🎯 Key Metrics

- **Total Sessions**: {patterns.get('total_sessions', 0)}
- **Total Events**: {patterns.get('total_events', 0)}
- **Average Session Duration**: {patterns.get('avg_session_duration', 0):.1f} seconds

## 🔄 Phase Usage Analysis

### Most Used Phases
"""
        
        # Add phase usage
        phase_usage = patterns.get('phase_usage', {})
        if phase_usage:
            for phase, count in phase_usage.most_common():
                content += f"- **{phase}**: {count} executions\n"
        else:
            content += "- No phase usage data available\n"
        
        content += "\n### Profile Usage Analysis\n"
        
        # Add profile usage
        profile_usage = patterns.get('profile_usage', {})
        if profile_usage:
            for profile, count in profile_usage.most_common():
                content += f"- **{profile}**: {count} uses\n"
        else:
            content += "- No profile usage data available\n"
        
        content += "\n## ⚠️ Common Issues & Recommendations\n"
        
        # Add common issues
        common_issues = patterns.get('common_issues', [])
        if common_issues:
            for issue in common_issues:
                priority_icon = "🔴" if issue.get('priority') == 'high' else "🟡"
                content += f"\n### {priority_icon} {issue.get('type', 'Unknown').title()}\n"
                content += f"- **Description**: {issue.get('description', 'No description')}\n"
                content += f"- **Recommendation**: {issue.get('recommendation', 'No recommendation')}\n"
                content += f"- **Priority**: {issue.get('priority', 'Unknown')}\n"
        else:
            content += "- No common issues identified\n"
        
        content += "\n## 📈 Performance Insights\n"
        
        # Add performance metrics
        performance_metrics = patterns.get('performance_metrics', {})
        if performance_metrics:
            for operation, durations in performance_metrics.items():
                if durations:
                    avg_duration = sum(durations) / len(durations)
                    min_duration = min(durations)
                    max_duration = max(durations)
                    content += f"\n### {operation.title()}\n"
                    content += f"- **Average Duration**: {avg_duration:.2f} seconds\n"
                    content += f"- **Range**: {min_duration:.2f}s - {max_duration:.2f}s\n"
                    content += f"- **Total Executions**: {len(durations)}\n"
                    
                    # Add performance recommendations
                    if avg_duration > 10.0:
                        content += f"- **⚠️ Performance Note**: This operation is taking longer than expected\n"
        else:
            content += "- No performance data available\n"
        
        content += "\n## 🎨 Asset Pattern Insights\n"
        
        # Add asset pattern insights
        asset_patterns = patterns.get('asset_patterns', {})
        if asset_patterns:
            for asset_type, properties_list in asset_patterns.items():
                content += f"\n### {asset_type.title()}\n"
                content += f"- **Total Assets Analyzed**: {len(properties_list)}\n"
                
                # Analyze common properties
                if properties_list:
                    # Get sample properties
                    sample_props = properties_list[0]
                    if isinstance(sample_props, dict):
                        content += "- **Common Properties**:\n"
                        for prop, value in sample_props.items():
                            if value is not None and value != "":
                                content += f"  - {prop}: {value}\n"
        else:
            content += "- No asset pattern data available\n"
        
        content += "\n## 🚀 Optimization Opportunities\n"
        
        # Generate asset-specific insights
        for asset_type in ['Texture2D', 'StaticMesh', 'Material']:
            insights = self.analyzer.generate_asset_insights(asset_type)
            if 'error' not in insights and insights.get('total_assets', 0) > 0:
                content += f"\n### {asset_type} Optimization\n"
                content += f"- **Total Assets**: {insights.get('total_assets', 0)}\n"
                
                # Add size statistics if available
                if 'size_stats' in insights:
                    size_stats = insights['size_stats']
                    content += f"- **Size Range**: {size_stats.get('min_dimension', 0)}px - {size_stats.get('max_dimension', 0)}px\n"
                    content += f"- **Average Size**: {size_stats.get('avg_dimension', 0):.0f}px\n"
                
                # Add optimization opportunities
                opportunities = insights.get('optimization_opportunities', [])
                if opportunities:
                    content += "- **Optimization Opportunities**:\n"
                    for opp in opportunities:
                        impact_icon = "🔴" if opp.get('impact') == 'high' else "🟡"
                        content += f"  - {impact_icon} {opp.get('description', 'No description')}\n"
                        content += f"    - **Recommendation**: {opp.get('recommendation', 'No recommendation')}\n"
        
        content += "\n## 📋 Action Items\n"
        
        # Generate action items based on analysis
        action_items = self._generate_action_items(patterns)
        for i, item in enumerate(action_items, 1):
            priority_icon = "🔴" if item.get('priority') == 'high' else "🟡"
            content += f"\n{i}. {priority_icon} **{item.get('title', 'Unknown')}**\n"
            content += f"   - **Description**: {item.get('description', 'No description')}\n"
            content += f"   - **Priority**: {item.get('priority', 'Unknown')}\n"
            content += f"   - **Effort**: {item.get('effort', 'Unknown')}\n"
        
        content += "\n## 📊 Data Quality Notes\n"
        
        # Add data quality information
        content += f"- **Report Generated**: {timestamp}\n"
        content += f"- **Data Source**: {self.knowledge_dir}\n"
        content += f"- **Total Events Analyzed**: {patterns.get('total_events', 0)}\n"
        
        if patterns.get('total_events', 0) < 10:
            content += "- **⚠️ Limited Data**: This report is based on limited usage data\n"
            content += "- **Recommendation**: Continue using the plugin to gather more insights\n"
        
        content += "\n---\n"
        content += "*This report is automatically generated by MagicOptimizer's self-learning system.*\n"
        content += "*Use these insights to guide plugin improvements and optimization strategies.*\n"
        
        return content
    
    def _generate_action_items(self, patterns: Dict[str, Any]) -> List[Dict[str, Any]]:
        """Generate actionable items based on analysis.
        
        Args:
            patterns: Analysis patterns dictionary
            
        Returns:
            List of action items
        """
        action_items = []
        
        # Check for high-priority issues
        common_issues = patterns.get('common_issues', [])
        for issue in common_issues:
            if issue.get('priority') == 'high':
                action_items.append({
                    'title': f"Fix {issue.get('type', 'Unknown')} Issue",
                    'description': issue.get('description', 'No description'),
                    'priority': 'high',
                    'effort': 'medium'
                })
        
        # Check for performance issues
        performance_metrics = patterns.get('performance_metrics', {})
        for operation, durations in performance_metrics.items():
            if durations:
                avg_duration = sum(durations) / len(durations)
                if avg_duration > 10.0:
                    action_items.append({
                        'title': f"Optimize {operation.title()} Performance",
                        'description': f"Operation taking {avg_duration:.1f}s on average",
                        'priority': 'medium',
                        'effort': 'high'
                    })
        
        # Check for usage patterns
        phase_usage = patterns.get('phase_usage', {})
        if phase_usage:
            # Identify underused phases
            total_usage = sum(phase_usage.values())
            for phase, count in phase_usage.items():
                usage_percentage = (count / total_usage) * 100 if total_usage > 0 else 0
                if usage_percentage < 10:  # Less than 10% usage
                    action_items.append({
                        'title': f"Improve {phase} Phase Adoption",
                        'description': f"Only {usage_percentage:.1f}% of operations use {phase}",
                        'priority': 'medium',
                        'effort': 'medium'
                    })
        
        # Add general improvement items
        if not action_items:
            action_items.append({
                'title': "Continue Data Collection",
                'description': "Gather more usage data to identify improvement opportunities",
                'priority': 'low',
                'effort': 'low'
            })
        
        return action_items


