"""
CSV input/output utilities for MagicOptimizer plugin
"""

import csv
import os
from pathlib import Path
from typing import List, Dict, Any, Optional, Union
import tempfile
import logging

logger = logging.getLogger(__name__)

def read_csv(file_path: str, encoding: str = 'utf-8') -> List[Dict[str, str]]:
    """
    Read CSV file and return list of dictionaries
    
    Args:
        file_path: Path to CSV file
        encoding: File encoding (default: utf-8)
        
    Returns:
        List of dictionaries representing CSV rows
    """
    if not os.path.exists(file_path):
        logger.warning(f"CSV file not found: {file_path}")
        return []
    
    data = []
    try:
        with open(file_path, 'r', newline='', encoding=encoding) as csvfile:
            reader = csv.DictReader(csvfile)
            for row in reader:
                data.append(row)
        logger.info(f"Successfully read {len(data)} rows from {file_path}")
    except Exception as e:
        logger.error(f"Error reading CSV {file_path}: {e}")
        return []
    
    return data

def _superset_fieldnames(preferred: List[str], rows: List[Dict[str, str]]) -> List[str]:
    """
    Build a stable superset of fieldnames, preserving the provided order and
    appending any extra keys found in rows at the end.
    """
    preferred_set = set(preferred)
    extras: List[str] = []
    for r in rows:
        for k in r.keys():
            if k not in preferred_set and k not in extras:
                extras.append(k)
    return list(preferred) + extras


def _normalize_row(row: Dict[str, str], fieldnames: List[str]) -> Dict[str, str]:
    return {k: row.get(k, "") for k in fieldnames}


def write_csv(file_path: str, fieldnames: List[str], rows: List[Dict[str, str]],
              encoding: str = 'utf-8') -> bool:
    """
    Write data to CSV file
    
    Args:
        file_path: Path to output CSV file
        fieldnames: List of column names
        rows: List of dictionaries representing CSV rows
        encoding: File encoding (default: utf-8)
        
    Returns:
        True if successful, False otherwise
    """
    try:
        # Ensure directory exists
        os.makedirs(os.path.dirname(file_path), exist_ok=True)

        # Tolerant header: include any unexpected keys present in rows
        effective_fieldnames = _superset_fieldnames(fieldnames, rows)
        normalized_rows = [_normalize_row(r, effective_fieldnames) for r in rows]

        # Atomic write via temp file then replace
        tmp_path = file_path + '.tmp'
        with open(tmp_path, 'w', newline='', encoding=encoding) as csvfile:
            writer = csv.DictWriter(csvfile, fieldnames=effective_fieldnames, dialect='excel', quoting=csv.QUOTE_MINIMAL, lineterminator='\n')
            writer.writeheader()
            writer.writerows(normalized_rows)
        os.replace(tmp_path, file_path)
        
        logger.info(f"Successfully wrote {len(rows)} rows to {file_path}")
        return True
    except Exception as e:
        logger.error(f"Error writing CSV {file_path}: {e}")
        return False

def validate_csv_header(file_path: str, expected_fields: List[str]) -> bool:
    """
    Validate CSV header contains expected fields
    
    Args:
        file_path: Path to CSV file
        expected_fields: List of expected field names
        
    Returns:
        True if all expected fields are present
    """
    if not os.path.exists(file_path):
        return False
    
    try:
        with open(file_path, 'r', newline='', encoding='utf-8') as csvfile:
            reader = csv.reader(csvfile)
            header = next(reader, [])
            return set(header) >= set(expected_fields)
    except Exception as e:
        logger.error(f"Error validating CSV header {file_path}: {e}")
        return False

def get_csv_field_value(row: Dict[str, str], field_name: str, default: str = "") -> str:
    """
    Get field value from CSV row with default fallback
    
    Args:
        row: Dictionary representing CSV row
        field_name: Name of field to retrieve
        default: Default value if field not found
        
    Returns:
        Field value or default
    """
    return row.get(field_name, default)

def append_to_csv(file_path: str, fieldnames: List[str], new_rows: List[Dict[str, str]],
                  encoding: str = 'utf-8') -> bool:
    """
    Append new rows to existing CSV file
    
    Args:
        file_path: Path to CSV file
        fieldnames: List of column names
        new_rows: New rows to append
        encoding: File encoding (default: utf-8)
        
    Returns:
        True if successful, False otherwise
    """
    try:
        # Ensure directory exists
        os.makedirs(os.path.dirname(file_path), exist_ok=True)

        # Determine existing header if file exists
        existing_header: Optional[List[str]] = None
        if os.path.exists(file_path):
            try:
                with open(file_path, 'r', newline='', encoding=encoding) as csvfile_r:
                    reader = csv.reader(csvfile_r)
                    existing_header = next(reader, None)
            except Exception as e:
                logger.warning(f"Could not read existing CSV header {file_path}: {e}")

        # Compute effective header as superset of provided + new row keys + existing header
        effective = list(fieldnames)
        if existing_header:
            for h in existing_header:
                if h not in effective:
                    effective.append(h)
        effective = _superset_fieldnames(effective, new_rows)
        normalized_new = [_normalize_row(r, effective) for r in new_rows]

        if existing_header is None:
            # New file: write header + rows
            tmp_path = file_path + '.tmp'
            with open(tmp_path, 'w', newline='', encoding=encoding) as csvfile:
                writer = csv.DictWriter(csvfile, fieldnames=effective, dialect='excel', quoting=csv.QUOTE_MINIMAL, lineterminator='\n')
                writer.writeheader()
                writer.writerows(normalized_new)
            os.replace(tmp_path, file_path)
        else:
            # Existing file. If headers match (set-wise), append safely; otherwise rewrite with unified header
            if set(existing_header) == set(effective):
                with open(file_path, 'a', newline='', encoding=encoding) as csvfile:
                    writer = csv.DictWriter(csvfile, fieldnames=effective, dialect='excel', quoting=csv.QUOTE_MINIMAL, lineterminator='\n')
                    writer.writerows(normalized_new)
            else:
                # Rewrite: read all old rows then write with new unified header
                try:
                    old_rows: List[Dict[str, str]] = []
                    with open(file_path, 'r', newline='', encoding=encoding) as csvfile_r:
                        reader = csv.DictReader(csvfile_r)
                        for row in reader:
                            old_rows.append(row)
                    normalized_old = [_normalize_row(r, effective) for r in old_rows]
                    tmp_path = file_path + '.tmp'
                    with open(tmp_path, 'w', newline='', encoding=encoding) as csvfile_w:
                        writer = csv.DictWriter(csvfile_w, fieldnames=effective, dialect='excel', quoting=csv.QUOTE_MINIMAL, lineterminator='\n')
                        writer.writeheader()
                        writer.writerows(normalized_old)
                        writer.writerows(normalized_new)
                    os.replace(tmp_path, file_path)
                except Exception as e:
                    logger.error(f"Error rewriting CSV with unified header {file_path}: {e}")
                    return False
        
        logger.info(f"Successfully appended {len(new_rows)} rows to {file_path}")
        return True
    except Exception as e:
        logger.error(f"Error appending to CSV {file_path}: {e}")
        return False

def merge_csv_files(input_files: List[str], output_file: str, 
                   fieldnames: Optional[List[str]] = None, encoding: str = 'utf-8') -> bool:
    """
    Merge multiple CSV files into one
    
    Args:
        input_files: List of input CSV file paths
        output_file: Output CSV file path
        fieldnames: Optional list of field names (auto-detected if None)
        encoding: File encoding (default: utf-8)
        
    Returns:
        True if successful, False otherwise
    """
    try:
        all_rows = []
        
        # Read all input files
        for input_file in input_files:
            if os.path.exists(input_file):
                rows = read_csv(input_file, encoding)
                all_rows.extend(rows)
                logger.info(f"Read {len(rows)} rows from {input_file}")
            else:
                logger.warning(f"Input file not found: {input_file}")
        
        if not all_rows:
            logger.warning("No data found in input files")
            return False
        
        # Auto-detect fieldnames if not provided
        if fieldnames is None:
            fieldnames = list(all_rows[0].keys())
        
        # Write merged data
        success = write_csv(output_file, fieldnames, all_rows, encoding)
        if success:
            logger.info(f"Successfully merged {len(all_rows)} rows to {output_file}")
        
        return success
    except Exception as e:
        logger.error(f"Error merging CSV files: {e}")
        return False

def create_csv_template(file_path: str, fieldnames: List[str], 
                       sample_data: Optional[List[Dict[str, str]]] = None,
                       encoding: str = 'utf-8') -> bool:
    """
    Create CSV template with headers and optional sample data
    
    Args:
        file_path: Path to output CSV file
        fieldnames: List of column names
        sample_data: Optional sample data rows
        encoding: File encoding (default: utf-8)
        
    Returns:
        True if successful, False otherwise
    """
    try:
        os.makedirs(os.path.dirname(file_path), exist_ok=True)
        
        with open(file_path, 'w', newline='', encoding=encoding) as csvfile:
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            
            if sample_data:
                writer.writerows(sample_data)
        
        logger.info(f"Successfully created CSV template {file_path}")
        return True
    except Exception as e:
        logger.error(f"Error creating CSV template {file_path}: {e}")
        return False

def get_csv_statistics(file_path: str) -> Dict[str, Any]:
    """
    Get basic statistics about CSV file
    
    Args:
        file_path: Path to CSV file
        
    Returns:
        Dictionary with file statistics
    """
    if not os.path.exists(file_path):
        return {"error": "File not found"}
    
    try:
        with open(file_path, 'r', newline='', encoding='utf-8') as csvfile:
            reader = csv.reader(csvfile)
            rows = list(reader)
            
            if not rows:
                return {"rows": 0, "columns": 0, "file_size": 0}
            
            stats = {
                "rows": len(rows) - 1,  # Exclude header
                "columns": len(rows[0]) if rows else 0,
                "file_size": os.path.getsize(file_path),
                "has_header": len(rows) > 0
            }
            
            return stats
    except Exception as e:
        logger.error(f"Error getting CSV statistics {file_path}: {e}")
        return {"error": str(e)}
