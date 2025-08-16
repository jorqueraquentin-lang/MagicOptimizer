"""
Pattern Analyzer for MagicOptimizer plugin.

Analyzes logged events to identify optimization patterns, common issues,
and insights that can improve the plugin's recommendations and performance.
"""

import os
import json
import csv
from collections import defaultdict, Counter
from typing import Dict, List, Any, Optional
from datetime import datetime, timedelta


class PatternAnalyzer:
    """Analyzes logged events to identify patterns and insights."""
    
    def __init__(self, knowledge_dir: str):
        """Initialize the pattern analyzer.
        
        Args:
            knowledge_dir: Path to the knowledge directory
        """
        self.knowledge_dir = knowledge_dir
        self.events_file = os.path.join(knowledge_dir, "events.jsonl")
        
    def analyze_usage_patterns(self) -> Dict[str, Any]:
        """Analyze overall usage patterns from logged events.
        
        Returns:
            Dictionary with usage pattern analysis
        """
        if not os.path.exists(self.events_file):
            return {"error": "No events file found"}
        
        patterns = {
            "total_sessions": 0,
            "total_events": 0,
            "phase_usage": Counter(),
            "profile_usage": Counter(),
            "error_patterns": Counter(),
            "performance_metrics": defaultdict(list),
            "asset_patterns": defaultdict(list),
            "ui_interactions": Counter()
        }
        
        try:
            with open(self.events_file, 'r', encoding='utf-8') as f:
                for line in f:
                    if line.strip():
                        event = json.loads(line)
                        self._process_event(event, patterns)
            
            # Calculate statistics
            patterns["total_events"] = len(patterns["total_events"])  # placeholder for future aggregation
            patterns["avg_session_duration"] = self._calculate_avg_session_duration()
            patterns["common_issues"] = self._identify_common_issues(patterns)
            
        except Exception as e:
            patterns["error"] = f"Analysis failed: {str(e)}"
        
        return patterns
    
    def _process_event(self, event: Dict[str, Any], patterns: Dict[str, Any]) -> None:
        """Process a single event to extract patterns.
        
        Args:
            event: Event dictionary
            patterns: Patterns dictionary to update
        """
        category = event.get("category", "")
        event_type = event.get("event_type", "")
        data = event.get("data", {})
        
        if category == "user_action":
            # Track phase and profile usage
            if "phase" in data:
                patterns["phase_usage"][data["phase"]] += 1
            if "profile" in data:
                patterns["profile_usage"][data["profile"]] += 1
                
        elif category == "optimization_result":
            # Track optimization results
            if "phase" in data:
                patterns["phase_usage"][data["phase"]] += 1
            if "profile" in data:
                patterns["profile_usage"][data["profile"]] += 1
                
        elif category == "error":
            # Track error patterns
            patterns["error_patterns"][event_type] += 1
            
        elif category == "performance":
            # Track performance metrics
            operation = data.get("operation", "unknown")
            duration = data.get("duration", 0)
            if duration > 0:
                patterns["performance_metrics"][operation].append(duration)
                
        elif category == "asset_pattern":
            # Track asset patterns
            asset_type = data.get("asset_type", "unknown")
            properties = data.get("properties", {})
            patterns["asset_patterns"][asset_type].append(properties)
            
        elif category == "ui_interaction":
            # Track UI interactions
            ui_element = data.get("ui_element", "unknown")
            action = data.get("action", "unknown")
            patterns["ui_interactions"][f"{ui_element}_{action}"] += 1
            
        elif category == "system" and event_type == "session_ended":
            # Track session completion
            patterns["total_sessions"] += 1
    
    def _calculate_avg_session_duration(self) -> float:
        """Calculate average session duration from events.
        
        Returns:
            Average session duration in seconds
        """
        durations = []
        
        try:
            with open(self.events_file, 'r', encoding='utf-8') as f:
                for line in f:
                    if line.strip():
                        event = json.loads(line)
                        if event.get("category") == "system" and event.get("event_type") == "session_ended":
                            duration = event.get("data", {}).get("session_duration", 0)
                            if duration > 0:
                                durations.append(duration)
        except Exception:
            pass
        
        return sum(durations) / len(durations) if durations else 0
    
    def _identify_common_issues(self, patterns: Dict[str, Any]) -> List[Dict[str, Any]]:
        """Identify common issues from error patterns and performance metrics.
        
        Args:
            patterns: Patterns dictionary
            
        Returns:
            List of common issues with recommendations
        """
        issues = []
        
        # Check for frequent errors
        for error_type, count in patterns["error_patterns"].most_common(5):
            if count > 1:
                issues.append({
                    "type": "error",
                    "description": f"Frequent {error_type} errors ({count} occurrences)",
                    "recommendation": f"Investigate root cause of {error_type} errors",
                    "priority": "high" if count > 5 else "medium"
                })
        
        # Check for performance issues
        for operation, durations in patterns["performance_metrics"].items():
            if durations:
                avg_duration = sum(durations) / len(durations)
                if avg_duration > 10.0:  # More than 10 seconds
                    issues.append({
                        "type": "performance",
                        "description": f"Slow {operation} operation (avg: {avg_duration:.1f}s)",
                        "recommendation": f"Optimize {operation} performance",
                        "priority": "medium"
                    })
        
        return issues
    
    def generate_asset_insights(self, asset_type: str) -> Dict[str, Any]:
        """Generate insights for a specific asset type.
        
        Args:
            asset_type: Type of asset to analyze
            
        Returns:
            Dictionary with asset insights
        """
        if not os.path.exists(self.events_file):
            return {"error": "No events file found"}
        
        insights = {
            "asset_type": asset_type,
            "total_assets": 0,
            "common_properties": defaultdict(Counter),
            "size_distribution": [],
            "format_distribution": Counter(),
            "optimization_opportunities": []
        }
        
        try:
            with open(self.events_file, 'r', encoding='utf-8') as f:
                for line in f:
                    if line.strip():
                        event = json.loads(line)
                        if (event.get("category") == "asset_pattern" and 
                            event.get("data", {}).get("asset_type") == asset_type):
                            
                            properties = event.get("data", {}).get("properties", {})
                            insights["total_assets"] += 1
                            
                            # Analyze common properties
                            for prop, value in properties.items():
                                if value is not None and value != "":
                                    insights["common_properties"][prop][str(value)] += 1
                            
                            # Analyze size distribution
                            if "width" in properties and "height" in properties:
                                try:
                                    width = int(properties["width"])
                                    height = int(properties["height"])
                                    if width > 0 and height > 0:
                                        insights["size_distribution"].append((width, height))
                                except (ValueError, TypeError):
                                    pass
                            
                            # Analyze format distribution
                            if "format" in properties:
                                format_val = properties["format"]
                                if format_val:
                                    insights["format_distribution"][str(format_val)] += 1
            
            # Calculate size statistics
            if insights["size_distribution"]:
                max_dimensions = [max(w, h) for w, h in insights["size_distribution"]]
                insights["size_stats"] = {
                    "max_dimension": max(max_dimensions),
                    "min_dimension": min(max_dimensions),
                    "avg_dimension": sum(max_dimensions) / len(max_dimensions)
                }
            
            # Identify optimization opportunities
            insights["optimization_opportunities"] = self._identify_asset_optimizations(insights)
            
        except Exception as e:
            insights["error"] = f"Analysis failed: {str(e)}"
        
        return insights
    
    def _identify_asset_optimizations(self, insights: Dict[str, Any]) -> List[Dict[str, Any]]:
        """Identify optimization opportunities for assets.
        
        Args:
            insights: Asset insights dictionary
            
        Returns:
            List of optimization opportunities
        """
        opportunities = []
        
        # Check for oversized textures
        if "size_stats" in insights:
            size_stats = insights["size_stats"]
            if size_stats["max_dimension"] > 4096:
                opportunities.append({
                    "type": "oversized",
                    "description": f"Textures larger than 4K found (max: {size_stats['max_dimension']}px)",
                    "recommendation": "Consider downscaling large textures for better performance",
                    "impact": "high"
                })
        
        # Check for uncompressed formats
        if "format_distribution" in insights:
            format_dist = insights["format_distribution"]
            uncompressed_formats = ["RGBA8", "BGRA8", "RGBA16F", "RGBA32F"]
            for fmt in uncompressed_formats:
                if fmt in format_dist and format_dist[fmt] > 0:
                    opportunities.append({
                        "type": "compression",
                        "description": f"Uncompressed {fmt} format found ({format_dist[fmt]} assets)",
                        "recommendation": f"Convert {fmt} to compressed format for target platform",
                        "impact": "medium"
                    })
        
        return opportunities
    
    def export_insights_csv(self, output_dir: str) -> str:
        """Export insights to CSV files for external analysis.
        
        Args:
            output_dir: Directory to save CSV files
            
        Returns:
            Path to the insights directory
        """
        insights_dir = os.path.join(output_dir, "insights")
        os.makedirs(insights_dir, exist_ok=True)
        
        # Export usage patterns
        patterns = self.analyze_usage_patterns()
        
        # Usage summary CSV
        usage_file = os.path.join(insights_dir, "usage_summary.csv")
        with open(usage_file, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(["Metric", "Value"])
            writer.writerow(["Total Sessions", patterns.get("total_sessions", 0)])
            writer.writerow(["Total Events", patterns.get("total_events", 0)])
            writer.writerow(["Avg Session Duration", f"{patterns.get('avg_session_duration', 0):.1f}s"]) 
            
            # Phase usage
            for phase, count in patterns.get("phase_usage", {}).most_common():
                writer.writerow([f"Phase: {phase}", count])
            
            # Profile usage
            for profile, count in patterns.get("profile_usage", {}).most_common():
                writer.writerow([f"Profile: {profile}", count])
        
        # Error patterns CSV
        errors_file = os.path.join(insights_dir, "error_patterns.csv")
        with open(errors_file, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(["Error Type", "Count"])
            for error_type, count in patterns.get("error_patterns", {}).most_common():
                writer.writerow([error_type, count])
        
        # Performance metrics CSV
        perf_file = os.path.join(insights_dir, "performance_metrics.csv")
        with open(perf_file, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(["Operation", "Count", "Avg Duration", "Min Duration", "Max Duration"])
            for operation, durations in patterns.get("performance_metrics", {}).items():
                if durations:
                    writer.writerow([
                        operation,
                        len(durations),
                        f"{sum(durations) / len(durations):.2f}s",
                        f"{min(durations):.2f}s",
                        f"{max(durations):.2f}s"
                    ])
        
        return insights_dir


