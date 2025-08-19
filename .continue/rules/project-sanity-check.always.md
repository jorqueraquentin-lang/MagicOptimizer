# Project Sanity Check and Goal Alignment - Always Apply

## 🎯 **MANDATORY: Regular Project Sanity Checks**

### **CRITICAL: Prevent Project Drift**
Regular sanity checks prevent the project from losing sight of its core objectives and ensure development stays on track.

### **Check Frequency Requirements**
- **DAILY (MANDATORY)**: Quick goal alignment check (5-10 minutes) - **REQUIRED EVERY DAY**
- **Weekly**: Medium-term progress review (15-30 minutes)  
- **Monthly**: Comprehensive long-term goal assessment (30-60 minutes)

## 📅 **Daily Sanity Check (MANDATORY - Every Day)**

### **When to Perform**
- **REQUIRED: Start of each development session**
- **REQUIRED: At least once per calendar day**
- **After completing a major task**
- **Before committing significant changes**

### **Daily Check Questions**
1. **Short-term Goal Alignment**:
   - ✅ Does today's work advance our immediate objectives?
   - ✅ Are we building toward the next milestone?
   - ✅ Is this work necessary for the current phase?

2. **Code Quality Check**:
   - ✅ Are we following established patterns?
   - ✅ Are we maintaining module boundaries?
   - ✅ Are we documenting our changes?

3. **Progress Validation**:
   - ✅ Are we closer to our short-term goals?
   - ✅ Are we solving the right problems?
   - ✅ Are we avoiding scope creep?

4. **Build Status Check**:
   - ✅ Does the plugin build successfully?
   - ✅ Does the plugin load in editor?
   - ✅ Are all features functional?
   - ✅ Are there any build warnings or errors?

## 📊 **Weekly Progress Review (Every 7 Days)**

### **When to Perform**
- **End of each week**
- **Before weekend breaks**
- **When switching between major phases**

### **Weekly Review Questions**
1. **Medium-term Goal Progress**:
   - ✅ Are we on track for this month's objectives?
   - ✅ Have we completed planned milestones?
   - ✅ Are we addressing technical debt appropriately?

2. **Development Velocity**:
   - ✅ Are we making steady progress?
   - ✅ Are we blocked on any critical issues?
   - ✅ Do we need to adjust our approach?

3. **Quality and Stability**:
   - ✅ Is the codebase becoming more stable?
   - ✅ Are we maintaining good test coverage?
   - ✅ Are we following best practices consistently?

4. **Build System Health**:
   - ✅ Is the plugin building consistently?
   - ✅ Are build times reasonable?
   - ✅ Are there recurring build issues?
   - ✅ Is the development build workflow working smoothly?

## 🗓️ **Monthly Goal Assessment (Every 30 Days)**

### **When to Perform**
- **End of each month**
- **Before major releases**
- **When planning next development cycle**

### **Monthly Assessment Questions**
1. **Long-term Goal Alignment**:
   - ✅ Are we building toward the ultimate project vision?
   - ✅ Have we achieved major milestones?
   - ✅ Are we on schedule for the final delivery?

2. **Strategic Direction**:
   - ✅ Is our technical approach still sound?
   - ✅ Are we addressing the right user needs?
   - ✅ Should we pivot or adjust our strategy?

3. **Resource and Timeline**:
   - ✅ Are we using our time effectively?
   - ✅ Do we need to adjust our timeline?
   - ✅ Are we managing technical debt appropriately?

4. **Build System Evolution**:
   - ✅ Is our build process efficient?
   - ✅ Are we using the right development workflow?
   - ✅ Do we need to optimize build times?
   - ✅ Are there build system improvements needed?

## 🔨 **MANDATORY: Build Status Validation**

### **Build Status Check Requirements**
```bash
# Every sanity check must include build validation:
1. Verify plugin builds successfully using direct development workflow
2. Confirm plugin loads in editor without crashes
3. Test core functionality works as expected
4. Check for any build warnings or errors
5. Document build status in sanity check log
```

### **Build Status Indicators**
- 🟢 **BUILD HEALTHY**: Plugin builds cleanly, loads in editor, all features functional
- 🟡 **BUILD WARNINGS**: Plugin builds but has warnings that should be addressed
- 🔴 **BUILD FAILED**: Plugin does not build or load in editor
- ⚠️ **BUILD DEGRADED**: Plugin builds but has issues affecting functionality

### **Build Status Check Process**
```bash
# Complete build status check:
1. Regenerate project files if needed
2. Build plugin using direct development build workflow
3. Verify plugin DLLs generated in correct location
4. Test plugin loads in editor
5. Verify UI panel opens and displays
6. Test core functionality
7. Check for UHT/UBT warnings
8. Document build status and any issues
```

## 📝 **MANDATORY: Sanity Check Logging**

### **Log Entry Format**
```markdown
### SANITY CHECK - [DATE] [TIME] - [CHECK_TYPE]

**CHECK TYPE**: Daily / Weekly / Monthly

**BUILD STATUS**: 🟢 HEALTHY / 🟡 WARNINGS / 🔴 FAILED / ⚠️ DEGRADED

**BUILD VALIDATION**:
- Plugin builds successfully: ✅/❌
- Plugin loads in editor: ✅/❌
- UI panel functional: ✅/❌
- Core features working: ✅/❌
- Build warnings: [List any warnings]
- Build errors: [List any errors]

**SHORT-TERM GOALS** (Next 1-7 days):
- [ ] Goal 1: Status and progress
- [ ] Goal 2: Status and progress
- [ ] Goal 3: Status and progress

**MEDIUM-TERM GOALS** (Next 1-4 weeks):
- [ ] Goal 1: Status and progress  
- [ ] Goal 2: Status and progress
- [ ] Goal 3: Status and progress

**LONG-TERM GOALS** (Next 1-6 months):
- [ ] Goal 1: Status and progress
- [ ] Goal 2: Status and progress
- [ ] Goal 3: Status and progress

**ALIGNMENT ASSESSMENT**:
- ✅ **ON TRACK**: What's working well
- ⚠️ **NEEDS ATTENTION**: What needs adjustment
- ❌ **OFF TRACK**: What needs immediate correction

**BUILD SYSTEM ASSESSMENT**:
- ✅ **WORKING WELL**: What's working in build system
- ⚠️ **NEEDS IMPROVEMENT**: What could be better
- ❌ **CRITICAL ISSUES**: What needs immediate fixing

**NEXT ACTIONS**:
- Action 1: Who, what, when
- Action 2: Who, what, when
- Action 3: Who, what, when

**OVERALL STATUS**: 🟢 ON TRACK / 🟡 NEEDS ATTENTION / 🔴 OFF TRACK
```

### **Logging Location and Organization**
- **Primary Location**: `docs/sanity_checks/` folder
- **File Naming**: `sanity_check_YYYY_MM_DD_HHMM.md` (timestamped)
- **Secondary Location**: `docs/DEVELOPMENT_LOG.md` (summary entry)
- **Format**: Use the exact format above
- **Timing**: Log immediately after completing the check

### **File Structure for Sanity Checks**
```
docs/
├── sanity_checks/
│   ├── sanity_check_2024_01_15_1430.md    # January 15, 2024 at 2:30 PM
│   ├── sanity_check_2024_01_16_0915.md    # January 16, 2024 at 9:15 AM
│   ├── sanity_check_2024_01_17_1600.md    # January 17, 2024 at 4:00 PM
│   └── ...
└── DEVELOPMENT_LOG.md                       # Summary entries
```

### **Daily Sanity Check Enforcement**
- **REQUIRED**: At least one sanity check per calendar day
- **REQUIRED**: File must be created in `docs/sanity_checks/` folder
- **REQUIRED**: File must be timestamped with date and time
- **REQUIRED**: Complete build status validation included
- **REQUIRED**: Summary entry added to `DEVELOPMENT_LOG.md`

## 🎯 **Goal Hierarchy Definition**

### **Short-term Goals (1-7 days)**
- **Immediate tasks** for current development phase
- **Bug fixes** and stability improvements
- **Documentation updates** for recent changes
- **Testing** of new features
- **Build system maintenance** and optimization

### **Medium-term Goals (1-4 weeks)**
- **Phase completion** (e.g., complete UI implementation)
- **Feature integration** and testing
- **Performance optimization** milestones
- **User experience improvements**
- **Build system improvements** and optimizations

### **Long-term Goals (1-6 months)**
- **Plugin completion** and packaging
- **User testing** and feedback integration
- **Performance benchmarks** achievement
- **Documentation** and user guides completion
- **Build system maturity** and efficiency

## ⚠️ **Warning Signs Requiring Immediate Attention**

### **Red Flags**
- ❌ **Scope Creep**: Adding features not in the original plan
- ❌ **Technical Debt**: Accumulating code quality issues
- ❌ **Goal Drift**: Working on tasks that don't advance objectives
- ❌ **Timeline Slippage**: Consistently missing milestones
- ❌ **Quality Degradation**: Breaking existing functionality
- ❌ **Build System Issues**: Plugin not building or loading consistently
- ❌ **Development Workflow Problems**: Inefficient or broken build process
- ❌ **Missing Daily Sanity Checks**: Not performing required daily checks

### **When to Escalate**
- **Immediate**: Any red flag detected
- **Same Day**: Document the issue and create action plan
- **Next Session**: Review progress on the action plan

## 🔄 **Sanity Check Workflow**

### **1. Preparation (2-3 minutes)**
- Review the last sanity check log entry
- Check current development log for recent progress
- Review project roadmap and milestones

### **2. Build Status Check (5-10 minutes)**
- Perform complete build validation
- Test plugin functionality in editor
- Document build status and any issues
- Identify build system improvements needed

### **3. Assessment (5-15 minutes)**
- Answer the relevant check questions
- Compare current state to planned objectives
- Identify any misalignments or issues
- Assess build system health and efficiency

### **4. Action Planning (3-5 minutes)**
- Define specific next actions
- Assign responsibility and timelines
- Update development priorities if needed
- Plan build system improvements if needed

### **5. Logging (2-3 minutes)**
- Create timestamped file in `docs/sanity_checks/` folder
- Document the sanity check results
- Add summary entry to `docs/DEVELOPMENT_LOG.md`
- Commit both files to version control

## 🎯 **Success Metrics**

- ✅ **Daily checks performed** at start of each session
- ✅ **Daily checks logged** in timestamped files in `docs/sanity_checks/`
- ✅ **Weekly reviews completed** every 7 days
- ✅ **Monthly assessments** every 30 days
- ✅ **All sanity checks logged** with complete information
- ✅ **Action items tracked** and completed
- ✅ **Project stays aligned** with long-term vision
- ✅ **Scope creep prevented** through regular validation
- ✅ **Technical debt managed** proactively
- ✅ **Build status validated** in every sanity check
- ✅ **Build system health** monitored and maintained
- ✅ **Development workflow** efficient and reliable

## 🚫 **What NOT to Do**

- ❌ **Skip daily sanity checks** - they are MANDATORY
- ❌ **Skip sanity checks** due to time constraints
- ❌ **Perform checks without logging** results
- ❌ **Ignore warning signs** or red flags
- ❌ **Continue development** when major misalignment detected
- ❌ **Make major changes** without goal validation
- ❌ **Skip build status validation** in sanity checks
- ❌ **Ignore build system issues** or warnings
- ❌ **Continue development** with broken build system
- ❌ **Create sanity check files** outside the `docs/sanity_checks/` folder

## ✅ **What to ALWAYS Do**

- ✅ **Perform daily sanity checks** - MANDATORY requirement
- ✅ **Create timestamped files** in `docs/sanity_checks/` folder
- ✅ **Log all results** in both locations (timestamped file + development log)
- ✅ **Take immediate action** on any red flags
- ✅ **Adjust priorities** based on check results
- ✅ **Communicate findings** to stakeholders if needed
- ✅ **Use check results** to guide next development session
- ✅ **Validate build status** in every sanity check
- ✅ **Monitor build system health** and efficiency
- ✅ **Address build issues** promptly and systematically
- ✅ **Maintain efficient development workflow** through regular checks

## 🔄 **Integration with Other Rules**

### **Build & Packaging Rules**
- **Build status validation** integrated into sanity checks
- **Development build workflow** verified regularly
- **Build system health** monitored continuously

### **Development Workflow Rules**
- **Build process integration** validated in sanity checks
- **Development efficiency** assessed regularly
- **Workflow improvements** identified and implemented

### **Testing & CI Rules**
- **Build validation** included in sanity check process
- **Testing results** reviewed in regular checks
- **CI system health** monitored and maintained

**Remember**: Regular sanity checks are not a waste of time - they're an investment in project success. A few minutes of reflection can save hours of misdirected work. Build status validation ensures your development foundation remains solid and efficient. **Daily sanity checks are MANDATORY and must be logged in timestamped files.**

description: Project sanity check rules that enforce regular goal alignment reviews, build status validation, and prevent project drift for the MagicOptimizer UE5 plugin development.
globs: ["**/*.md", "**/*.py", "**/*.ps1", "**/*.bat"]
alwaysApply: true
---
