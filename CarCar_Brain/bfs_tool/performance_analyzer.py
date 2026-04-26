import pandas as pd
import glob
import io
import os

def run_performance_analysis():
    # 取得當前腳本目錄並設定路徑
    script_dir = os.path.dirname(os.path.abspath(__file__))
    data_folder = os.path.join(script_dir, "running_data")
    output_file = os.path.join(script_dir, "Carcar_Performance_Report.xlsx")
    
    all_files = glob.glob(os.path.join(data_folder, "*.txt"))
    if not all_files:
        print(f"Cannot find any .txt files in {data_folder}.")
        return

    all_dataframes = []
    run_counter = 1  # 🌟 新增：全域的跑圖次數計數器
    
    for filename in all_files:
        with open(filename, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
        
        clean_lines = []
        is_capturing = False
        
        for line in lines:
            if "--- FINAL REPORT ---" in line:
                is_capturing = True
                clean_lines = []  # 🌟 新增：每次看到開頭，就清空上一份報表的暫存，準備接新數據
                continue
                
            if "--- END REPORT ---" in line:
                is_capturing = False
                # 🌟 關鍵修改：一看到結束標記，就立刻把剛才收集到的 clean_lines 結算成一次 Run
                if len(clean_lines) > 0:
                    csv_data = "\n".join(clean_lines)
                    try:
                        column_names = ['Action', 'Count', 'TotalTime(ms)', 'MaxTime(ms)', 'MinTime(ms)', 'AvgTime(ms)']
                        df = pd.read_csv(io.StringIO(csv_data), names=column_names)
                        
                        numeric_cols = ['Count', 'TotalTime(ms)', 'MaxTime(ms)', 'MinTime(ms)', 'AvgTime(ms)']
                        for col in numeric_cols:
                            df[col] = pd.to_numeric(df[col], errors='coerce')
                        df = df.dropna()
                        
                        # 使用全域計數器來命名 Run_ID
                        df.insert(0, 'Run_ID', f"Run_{run_counter}")
                        all_dataframes.append(df)
                        
                        run_counter += 1  # 結算成功，跑圖次數 +1
                    except Exception as e:
                        print(f"⚠️ Error parsing a report in file {filename}: {e}")
                
                # 結算完不要 break，繼續往下讀，看看檔案裡還有沒有下一次 FINAL REPORT！
                continue 
                
            if is_capturing and line.strip():
                if line.startswith("Action,Count"):
                    continue
                clean_lines.append(line.strip())

    if not all_dataframes: 
        print("❌ Failed to parse any valid data.")
        return

    # 合併所有跑圖資料
    merged_df = pd.concat(all_dataframes, ignore_index=True)
    
    # 計算各動作的綜合數據
    analysis_summary = merged_df.groupby('Action').agg(
        Runs_Count=('Run_ID', 'nunique'),
        Total_Executions=('Count', 'sum'),
        Total_Time_Sum=('TotalTime(ms)', 'sum'),
        Time_Fluctuation_Std=('AvgTime(ms)', 'std'),
        Max_Time_Ever=('MaxTime(ms)', 'max'),
        Min_Time_Ever=('MinTime(ms)', 'min')
    ).reset_index()

    # 計算真正的加權平均
    analysis_summary['Weighted_AvgTime(ms)'] = analysis_summary['Total_Time_Sum'] / analysis_summary['Total_Executions']

    # 🌟 新增：自訂強制排序，讓報表更符合人類閱讀邏輯
    custom_order = [
        'Track_150', 'Track_200', 'Track_Accel', 'Track_Decel', 
        'Track_UTurn_in', 'Track_UTurn_out',
        'Turn_F', 'Turn_L', 'Turn_R', 
        'Turn_T', 'Turn_B', 
        'Turn_LB', 'Turn_RB'
    ]
    # 將 Action 轉換為自訂分類並排序
    analysis_summary['Action'] = pd.Categorical(analysis_summary['Action'], categories=custom_order, ordered=True)
    analysis_summary = analysis_summary.sort_values('Action')

    # 重新排列欄位順序並四捨五入
    analysis_summary = analysis_summary[[
        'Action', 
        'Runs_Count', 
        'Total_Executions', 
        'Weighted_AvgTime(ms)', 
        'Time_Fluctuation_Std', 
        'Max_Time_Ever', 
        'Min_Time_Ever'
    ]].round(2)

    # 寫入 Excel
    with pd.ExcelWriter(output_file, engine='openpyxl') as writer:
        merged_df.to_excel(writer, sheet_name='All_Runs_Data', index=False)
        analysis_summary.to_excel(writer, sheet_name='Analysis_Summary', index=False)
    
    print(f"✅ Analysis complete! Processed {run_counter - 1} runs. Weighted report saved to: {output_file}")

if __name__ == "__main__":
    run_performance_analysis()