class VideoCategoriesController < ApplicationController
  def index
    list
    render :action => 'list'
  end

  # GETs should be safe (see http://www.w3.org/2001/tag/doc/whenToUseGet.html)
  verify :method => :post, :only => [ :destroy, :create, :update ],
         :redirect_to => { :action => :list }

  def list
    @video_category_pages, @video_categories = paginate :video_categories, :per_page => 10
  end

  def show
    @video_category = VideoCategory.find(params[:id])
  end

  def new
    @video_category = VideoCategory.new
  end

  def create
    @video_category = VideoCategory.new(params[:video_category])
    if @video_category.save
      flash[:notice] = 'VideoCategory was successfully created.'
      redirect_to :action => 'list'
    else
      render :action => 'new'
    end
  end

  def edit
    @video_category = VideoCategory.find(params[:id])
  end

  def update
    @video_category = VideoCategory.find(params[:id])
    if @video_category.update_attributes(params[:video_category])
      flash[:notice] = 'VideoCategory was successfully updated.'
      redirect_to :action => 'show', :id => @video_category
    else
      render :action => 'edit'
    end
  end

  def destroy
    VideoCategory.find(params[:id]).destroy
    redirect_to :action => 'list'
  end
end
