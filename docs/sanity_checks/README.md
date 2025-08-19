# Sanity Checks Folder

This folder contains timestamped sanity check reports that are **MANDATORY** for the MagicOptimizer project development.

## 📅 **Daily Sanity Check Requirement**

**MANDATORY**: At least one sanity check must be performed and logged **every calendar day**.

## 📁 **File Naming Convention**

All sanity check files must follow this naming pattern:
```
sanity_check_YYYY_MM_DD_HHMM.md
```

**Examples:**
- `sanity_check_2024_01_15_1430.md` - January 15, 2024 at 2:30 PM
- `sanity_check_2024_01_16_0915.md` - January 16, 2024 at 9:15 AM
- `sanity_check_2024_01_17_1600.md` - January 17, 2024 at 4:00 PM

## 📝 **Required Content**

Each sanity check file must include:

1. **Build Status Validation** - Plugin build and functionality test
2. **Goal Alignment Assessment** - Short, medium, and long-term goals
3. **Progress Tracking** - What's working and what needs attention
4. **Action Items** - Specific next steps with timelines
5. **Overall Status** - 🟢 ON TRACK / 🟡 NEEDS ATTENTION / 🔴 OFF TRACK

## 🔄 **Workflow**

1. **Perform the sanity check** (5-10 minutes daily)
2. **Create timestamped file** in this folder
3. **Add summary entry** to `../DEVELOPMENT_LOG.md`
4. **Commit both files** to version control

## ⚠️ **Important Notes**

- **Daily checks are MANDATORY** - no exceptions
- **Build status must be validated** in every check
- **Files must be timestamped** with date and time
- **Both locations must be updated** (this folder + development log)
- **Missing daily checks are a red flag** requiring immediate attention

## 📊 **Check Types**

- **Daily**: Quick goal alignment (5-10 minutes) - **REQUIRED EVERY DAY**
- **Weekly**: Medium-term progress review (15-30 minutes)
- **Monthly**: Comprehensive assessment (30-60 minutes)

## 🎯 **Purpose**

Regular sanity checks prevent project drift, ensure goal alignment, validate build system health, and maintain development efficiency. They are an investment in project success.

---

*For complete rules and requirements, see `../project-sanity-check.always.mdc`*
