# Development Logging Requirements - Always Apply

## 🎯 **Scope and Purpose**

This rule enforces detailed development log requirements, format standards, and content requirements for MagicOptimizer development.

## 📝 **Log Content Requirements**

### **Mandatory Content Elements**
- **Long and detailed** development logs (as per user preference)
- **Detailed presentation** of next steps to cover
- **Project's inspirational roadmap** included verbatim
- **Timestamped entries** for all development activities
- **Before/after states** for all changes
- **Root cause analysis** for all fixes

### **Log Entry Structure**
```
## [Date] - [Task Title]

### **What Was Done**
- Detailed description of changes made
- Files modified and why
- Technical approach taken

### **Results and Validation**
- What was achieved
- How it was tested
- Any issues encountered

### **Next Steps**
- Detailed presentation of what should be done next
- Specific tasks and priorities
- Dependencies and blockers

### **Roadmap Context**
[Include relevant section of project roadmap verbatim]
```

## 🗺️ **Roadmap Integration**

### **Roadmap Inclusion Requirements**
- **Always include** the project's inspirational roadmap in development logs
- **Quote verbatim** - do not paraphrase or summarize
- **Reference specific sections** relevant to current work
- **Update roadmap references** as project evolves

### **Roadmap Sections to Include**
- **Long-term goals** (6+ months)
- **Mid-term objectives** (3-6 months)
- **Short-term targets** (1-3 months)
- **Current sprint focus** (1-2 weeks)

## 📊 **Log Quality Standards**

### **Detail Level Requirements**
- **Comprehensive coverage** of all development activities
- **Technical depth** appropriate for the audience
- **Context and reasoning** for all decisions
- **Alternative approaches** considered and rejected
- **Lessons learned** and insights gained

### **Formatting Standards**
- **Clear headings** and subheadings
- **Bullet points** for lists and steps
- **Code blocks** for technical content
- **Links and references** to related documentation
- **Visual elements** when helpful (screenshots, diagrams)

## 🔄 **Log Maintenance**

### **Update Frequency**
- **After every task** completion
- **After every significant change**
- **After every testing session**
- **After every milestone** achievement
- **Weekly progress summaries**

### **Content Organization**
- **Chronological order** by date
- **Logical grouping** by feature or module
- **Cross-references** between related entries
- **Searchable content** with clear keywords
- **Version tracking** for major changes

## 🚫 **Log Anti-Patterns**

### **What NOT to Do**
- ❌ **Don't write brief** or superficial entries
- ❌ **Don't skip next steps** planning
- ❌ **Don't omit roadmap** context
- ❌ **Don't forget timestamps** or dates
- ❌ **Don't ignore validation** results

### **What to ALWAYS Do**
- ✅ **Write detailed** and comprehensive entries
- ✅ **Include next steps** with specific details
- ✅ **Reference roadmap** sections verbatim
- ✅ **Document all changes** and decisions
- ✅ **Update after every** development activity

## 🎯 **Success Metrics**

- ✅ **All development activities** logged in detail
- ✅ **Next steps clearly defined** for each entry
- ✅ **Roadmap sections included** verbatim
- ✅ **Logs updated** after every task
- ✅ **Content searchable** and well-organized
- ✅ **Technical depth** appropriate for audience

description: Development logging rules that enforce detailed log requirements, roadmap integration, and comprehensive documentation for MagicOptimizer development.
globs: ["**/*.md", "**/*.py", "**/*.txt"]
alwaysApply: true
---
